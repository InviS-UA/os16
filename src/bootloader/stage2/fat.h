#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

typedef struct
{
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreationTimeHundredths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t LastAccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t LastModificationTime;
    uint16_t LastModificationDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) FAT_DirectoryEntry;

enum FAT_Attributes
{
    FAT_ATTR_READ_ONLY  = 0x01,
    FAT_ATTR_HIDDEN     = 0x02,
    FAT_ATTR_SYSTEM     = 0x04,
    FAT_ATTR_VOLUME_ID  = 0x08,
    FAT_ATTR_DIRECTORY  = 0x10,
    FAT_ATTR_ARCHIVE    = 0x20,
    FAT_ATTR_LFN        = FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID
};

typedef struct
{
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;

bool FAT_Init(DISK* disk);
FAT_File __far* FAT_Open(DISK* disk, const char* path);
uint32_t FAT_Read(DISK* disk, FAT_File __far* file, uint32_t size, void __far* dataOut);
bool FAT_ReadEntry(DISK* disk, FAT_File __far* file, FAT_DirectoryEntry* entryOut);
void FAT_Close(FAT_File __far* file);