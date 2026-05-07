#include "memory.h"

void far* memcpy(void far* dst, const void far* src, size_t num)
{
    uint8_t far* u8Dst = (uint8_t far*)dst;
    const uint8_t far* u8Src = (const uint8_t far*)src;

    for (size_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void far* memset(void far* ptr, int value, size_t num)
{
    uint8_t far* u8Ptr = (uint8_t far*)ptr;

    for (size_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void far* ptr1, const void far* ptr2, size_t num)
{
    const uint8_t far* u8Ptr1 = (const uint8_t far*)ptr1;
    const uint8_t far* u8Ptr2 = (const uint8_t far*)ptr2;

    for (size_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}