#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "memdefs.h"
#include "fat.h"
#include "memory.h"
#include "defs.h"

uint8_t __far* g_Kernel = (uint8_t __far*)MEMORY_KERNEL_ADDR;
uint8_t __far* g_KernelLoadBuffer = (uint8_t __far*)MEMORY_LOAD_ADDR;

typedef void (*KernelEntry)(uint16_t) __far;

void ASMFUNC cstart_(uint16_t bootDrive)
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

    FAT_File __far* file = FAT_Open(&disk, "kernel.sys");
    uint32_t read;
    uint8_t __far* kernelBuffer = g_Kernel;
    printf("Loading KERNEL.SYS\r\n");

    while ((read = FAT_Read(&disk, file, MEMORY_LOAD_SIZE, g_KernelLoadBuffer)))
    {
        memcpy(kernelBuffer, g_KernelLoadBuffer, read);
        kernelBuffer += read;
    }

    FAT_Close(file);

    KernelEntry entry = (KernelEntry)g_Kernel;
    entry(bootDrive);

end:
    for (;;);
}