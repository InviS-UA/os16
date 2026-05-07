#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive)
{
    DISK disk;

    if (!DISK_Init(&disk, bootDrive))
    {
        printf("Cannot init disk!\r\n");
        goto end;
    }

    if (!FAT_Init(&disk))
    {
        printf("Cannot init FAT!\r\n");
        goto end;
    }

    FAT_File far* file = FAT_Open(&disk, "test.txt");
    char buff[100];

    uint32_t read = FAT_Read(&disk, file, sizeof(buff), buff);

    for (int i = 0; i < read; i++)
        putc(buff[i]);

    FAT_Close(file);

end:
    for (;;);
}