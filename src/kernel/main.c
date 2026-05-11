#include <stdint.h>
#include "stdio.h"

void __attribute__((section(".entry"))) kmain(uint16_t bootDrive)
{
    printf("Hello world from kernel!\r\n");

end:
    for (;;);
}