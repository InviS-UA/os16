#include <stdint.h>
#include "stdio.h"
#include "disk.h"

uint8_t far* data = (uint8_t far*)0x20000000;

void _cdecl cstart_(uint16_t bootDrive)
{
    printf("Hello world from C!\r\n");

    DISK disk;

    if (!DISK_Init(&disk, bootDrive))
    {
        printf("Cannot init disk!\r\n");
        goto end;
    }

    if (!DISK_ReadSectors(&disk, 0, 1, data))
    {
        printf("Cannot read from disk!\r\n");
        goto end;
    }

    for (int i = 0; i < 512; i++)
        putc(data[i]);

end:
    for (;;);
}