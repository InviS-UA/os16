#pragma once

#include <stdint.h>
#include <stddef.h>

#include "defs.h"

void __far* memcpy(void __far* dst, const void __far* src, size_t num);
void __far* memset(void __far* ptr, int value, size_t num);
int memcmp(const void __far* ptr1, const void __far* ptr2, size_t num);