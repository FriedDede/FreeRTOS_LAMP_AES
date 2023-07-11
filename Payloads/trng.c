#include <stdint.h>
#include "trng.h"

uint32_t rand;

int trng()
{
    #ifndef QEMU
    int i;
    uint32_t rand;
    int address = 0x400000;
    __asm__ volatile (
        "lw %0, 0(%1)"
        : "=r" (rand)
        : "rm" (0x400000)
    );
    return rand;
    #endif
}
