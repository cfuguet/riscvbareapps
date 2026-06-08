#include <stdio.h>
#include "bsp/bsp_cmo.h"
#include "common/compiler.h"
#include "common/cpu.h"

#define CACHE_BLOCK_SIZE 64
#define __align_to_cacheblock __align(CACHE_BLOCK_SIZE)

__align_to_cacheblock uint32_t blk = 0xdeadbeeful;

int main()
{
    printf("[ BEGIN ] CMO test\n");
    printf("> blk = %lu\n", blk);

    cpu_mfence();
    blk = 5;
    printf("> blk = %lu\n", blk);

    cpu_mfence();
    cmo_inval(blk);
    printf("> blk = %lu\n", blk);

    printf("[ END   ] CMO test\n");
    return 0;
}
