#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "defs.h"

typedef struct
{
    uint8_t drive;
} DISK;

bool DISK_Init(DISK* disk, uint8_t driveNumber);
bool DISK_ReadSectors(DISK* disk, uint64_t lba, uint16_t sectors, void far* dataOut);