#pragma once

#include <stdint.h>
#include <stdbool.h>

void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);

typedef struct
{
    uint8_t size;
    uint8_t unused;
    uint16_t sectors;
    uint8_t far* ptr;
    uint64_t lba;
} __attribute__((packed)) x86_disk_dap;

bool _cdecl x86_Disk_CheckExetensionsPresent(uint8_t drive);
bool _cdecl x86_Disk_ExtendedRead(uint8_t drive, x86_disk_dap* dap);
bool _cdecl x86_Disk_Reset(uint8_t drive);