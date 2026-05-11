#include "fat.h"
#include <stddef.h>
#include "memdefs.h"
#include "stdio.h"
#include "memory.h"
#include "ctype.h"
#include "string.h"

#define SECTOR_SIZE             512
#define MAX_PATH_SIZE           256
#define MAX_FILE_HANDLES        10
#define ROOT_DIRECTORY_HANDLE   -1

typedef struct
{
    uint8_t BootJump[3];
    uint8_t Oem[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntriesCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    // exetended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} __attribute__((packed)) FAT_BootSector;

typedef struct
{
    uint8_t Buffer[SECTOR_SIZE];
    FAT_File Public;
    bool Opened;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint32_t FirstCluster;
} FAT_FileData;

typedef struct
{
    union
    {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;

    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t far* g_Fat = NULL;
static uint32_t g_DataSectionLBA;

static bool FAT_ReadBootSector(DISK* disk)
{
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

static bool FAT_ReadFat(DISK* disk)
{
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_Init(DISK* disk)
{
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    // read boot sector
    if (!FAT_ReadBootSector(disk))
    {
        printf("FAT: read boot sector failed!\r\n");
        return false;
    }

    // read FAT
    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;

    if (sizeof(g_Data) + fatSize >= MEMORY_FAT_SIZE)
    {
        printf("FAT: not enough memory to read FAT! Required %lu, only have %u\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!FAT_ReadFat(disk))
    {
        printf("FAT: read fat failed!\r\n");
        return false;
    }

    // open root directory file
    uint32_t rootDirLBA = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;

    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = rootDirSize;
    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.FirstCluster = rootDirLBA;
    g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!DISK_ReadSectors(disk, rootDirLBA, 1, g_Data->RootDirectory.Buffer))
    {
        printf("FAT: read root directory failed\r\n");
        return false;
    }

    // calcualate data section
    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLBA = rootDirLBA + rootDirSectors;

    // reset opened files
    for (size_t i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].Opened = false;

    return true;
}

static uint32_t FAT_ClusterToLBA(uint32_t cluster)
{
    return (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster + g_DataSectionLBA;
}

static uint32_t FAT_NextCluster(uint32_t currentCluster)
{
    uint32_t fatIndex = currentCluster * 2;

    uint32_t nextCluster = *(uint16_t far*)(g_Fat + fatIndex);

    if (nextCluster >= 0xFFF8)
        nextCluster |= 0xFFFF0000;

    return nextCluster;
}

static FAT_File far* FAT_OpenEntry(DISK* disk, FAT_DirectoryEntry* entry)
{
    int handle = -1;

    for (size_t i = 0; i < MAX_FILE_HANDLES; i++)
        if (!g_Data->OpenedFiles[i].Opened)
            handle = i;

    if (handle == -1)
    {
        printf("FAT: Out of file handles!\r\n");
        return false;
    }

    FAT_FileData far* fd = &g_Data->OpenedFiles[handle];

    fd->Public.Handle = handle;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTR_DIRECTORY) != 0;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    // printf("OpenEntry FirstCluster: %lx, CurrentCluster: %lx\r\n", fd->FirstCluster, fd->CurrentCluster);

    if (!DISK_ReadSectors(disk, FAT_ClusterToLBA(fd->CurrentCluster), 1, fd->Buffer))
    {
        printf("FAT: read error!\r\n");
        return false;
    }

    fd->Opened = true;

    return &fd->Public;
}

uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t size, void far* dataOut)
{
    FAT_FileData far* fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
        ? &g_Data->RootDirectory
        : &g_Data->OpenedFiles[file->Handle];

    uint8_t far* u8DataOut = (uint8_t far*)dataOut;

    if (!fd->Public.IsDirectory || (fd->Public.IsDirectory && fd->Public.Size > 0))
        size = min(size, fd->Public.Size - fd->Public.Position);

    // printf("Read FirstCluster: %lx, CurrentCluster: %lx\r\n", fd->FirstCluster, fd->CurrentCluster);

    while (size > 0)
    {
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(size, leftInBuffer);

        memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8DataOut += take;
        fd->Public.Position += take;
        size -= take;

        if (leftInBuffer == take)
        {
            if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE)
            {
                ++fd->CurrentCluster;

                // read next sector
                if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer))
                {
                    printf("FAT: read error!\r\n");
                    break;
                }
            }
            else
            {
                // calculate next cluster & cluster to read
                if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster)
                {
                    fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
                    fd->CurrentSectorInCluster = 0;
                }

                if (fd->CurrentCluster >= 0xFFF8)
                {
                    // Mark end of file
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                // read next sector
                if (!DISK_ReadSectors(disk, FAT_ClusterToLBA(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer))
                {
                    printf("FAT: read error!\r\n");
                    break;
                }
            }
        }
    }

    return u8DataOut - (uint8_t far*)dataOut;
}

bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* entryOut)
{
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), entryOut) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File far* file)
{
    if (file->Handle == ROOT_DIRECTORY_HANDLE)
    {
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    }
    else
    {
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

bool FAT_FindFile(DISK* disk, FAT_File far* file, const char* name, FAT_DirectoryEntry* entryOut)
{
    char fatName[12];
    FAT_DirectoryEntry entry;

    // convert from name to fat name
    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';

    const char* ext = strchr(name, '.');
    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
        fatName[i] = toupper(name[i]);

    if (ext != NULL)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            fatName[i + 8] = toupper(ext[i + 1]);
    }

    while (FAT_ReadEntry(disk, file, &entry))
    {
        if (memcmp(fatName, entry.Name, 11) == 0)
        {
            memcpy(entryOut, &entry, sizeof(FAT_DirectoryEntry));
            return true;
        }
    }
    
    return false;
}

FAT_File far* FAT_Open(DISK* disk, const char* path)
{
    char name[MAX_PATH_SIZE];

    // ignore leading slash
    if (path[0] == '/' || path[0] == '\\')
        path++;

    FAT_File far* current = &g_Data->RootDirectory.Public;

    while (*path)
    {
        // extract next file name from path
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (!delim)
            delim = strchr(path, '\\');

        if (delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }
        
        // find directory entry in current directory
        FAT_DirectoryEntry entry;
        if (FAT_FindFile(disk, current, name, &entry))
        {
            FAT_Close(current);

            // check if directory
            if (!isLast && entry.Attributes & FAT_ATTR_DIRECTORY == 0)
            {
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }

            // open new directory entry
            current = FAT_OpenEntry(disk, &entry);
        }
        else
        {
            FAT_Close(current);

            printf("FAT: %s not found\r\n", name);
            return NULL;
        }
    }

    return current;
}