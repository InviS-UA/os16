#pragma once

#include <stdint.h>
#include <stddef.h>

#include "defs.h"

void far* memcpy(void far* dst, const void far* src, size_t num);
void far* memset(void far* ptr, int value, size_t num);
int memcmp(const void far* ptr1, const void far* ptr2, size_t num);