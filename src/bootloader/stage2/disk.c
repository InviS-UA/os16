#include "disk.h"
#include "x86.h"
#include <stdio.h>

bool DISK_Init(DISK* disk, uint8_t driveNumber)
{
    if (!x86_Disk_CheckExetensionsPresent(driveNumber))
        return false;

    disk->drive = driveNumber;

    return true;
}

bool DISK_ReadSectors(DISK* disk, uint64_t lba, uint16_t sectors, void __far* dataOut)
{
    x86_disk_dap dap = {
        .size = sizeof(x86_disk_dap),
        .unused = 0,
        .sectors = sectors,
        .ptr = dataOut,
        .lba = lba
    };

    for (int i = 0; i < 3; i++)
    {
        if (x86_Disk_ExtendedRead(disk->drive, &dap))
            return true;

        x86_Disk_Reset(disk->drive);
    }

    return false;
}