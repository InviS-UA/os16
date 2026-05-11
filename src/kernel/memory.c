#include "memory.h"

void __far* memcpy(void __far* dst, const void __far* src, size_t num)
{
    uint8_t __far* u8Dst = (uint8_t __far*)dst;
    const uint8_t __far* u8Src = (const uint8_t __far*)src;

    for (size_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void __far* memset(void __far* ptr, int value, size_t num)
{
    uint8_t __far* u8Ptr = (uint8_t __far*)ptr;

    for (size_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void __far* ptr1, const void __far* ptr2, size_t num)
{
    const uint8_t __far* u8Ptr1 = (const uint8_t __far*)ptr1;
    const uint8_t __far* u8Ptr2 = (const uint8_t __far*)ptr2;

    for (size_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}