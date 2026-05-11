#pragma once

#include "defs.h"

#define MEMORY_FAT_ADDR      ((void far*)0x20000000)
#define MEMORY_FAT_SIZE      0x10000

#define MEMORY_LOAD_ADDR     ((void far*)0x30000000)
#define MEMORY_LOAD_SIZE     0x10000

#define MEMORY_KERNEL_ADDR   ((void far*)0x00007E00)