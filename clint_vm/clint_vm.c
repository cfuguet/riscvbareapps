/*  Copyright 2024 Univ. Grenoble Alpes, Inria, TIMA
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/* 
 *  Author: Cesar Fuguet
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common/cpu.h"
#include "common/trap_handler.h"
#include "bsp/bsp_irq.h"

#ifndef TIMER_PERIOD
#define TIMER_PERIOD 512
#endif

volatile int counter;

void init_vm(uintptr_t start_func);

//  clint handler
uintptr_t clint_timer_handler(uintptr_t mcause, uintptr_t mstatus, uintptr_t mepc)
{
    printf("timer_handler: %d\n", counter++);
    clint_set_mtime(bsp_get_clint_driver(cpu_id()), 0);
    return mepc;
}

//  entry point after VM initialization (vm_init)
void __main()
{
    volatile uint64_t *data;

    printf("Start clint_vm\n");

    counter = 0;

    //  initialize data
    data = (volatile uint64_t*)malloc(8*4096);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            data[((i << 12) >> 3) + j] = (volatile uint64_t)&data[((i << 12) >> 3) + j];
        }
    }

    printf("test vector @%x\n", data);

    //  check written data
    for (int iter = 0; iter < 512; iter++) {
        uint64_t d0, d1, d2, d3, d4, d5, d6, d7;
        int retries = rand() & 0xf;
        for (volatile int retry = 0; retry < retries; retry++) {
            d0 = (volatile uint64_t)data[((0 << 12) >> 3) + 0];
            d1 = (volatile uint64_t)data[((1 << 12) >> 3) + 0];
            d2 = (volatile uint64_t)data[((2 << 12) >> 3) + 0];
            d3 = (volatile uint64_t)data[((3 << 12) >> 3) + 0];
            d4 = (volatile uint64_t)data[((4 << 12) >> 3) + 0];
            d5 = (volatile uint64_t)data[((5 << 12) >> 3) + 0];
            d6 = (volatile uint64_t)data[((6 << 12) >> 3) + 0];
            d7 = (volatile uint64_t)data[((7 << 12) >> 3) + 0];
        }
        if (d0 != (volatile uint64_t)&data[((0 << 12) >> 3) + 0]) goto error;
        if (d1 != (volatile uint64_t)&data[((1 << 12) >> 3) + 0]) goto error;
        if (d2 != (volatile uint64_t)&data[((2 << 12) >> 3) + 0]) goto error;
        if (d3 != (volatile uint64_t)&data[((3 << 12) >> 3) + 0]) goto error;
        if (d4 != (volatile uint64_t)&data[((4 << 12) >> 3) + 0]) goto error;
        if (d5 != (volatile uint64_t)&data[((5 << 12) >> 3) + 0]) goto error;
        if (d6 != (volatile uint64_t)&data[((6 << 12) >> 3) + 0]) goto error;
        if (d7 != (volatile uint64_t)&data[((7 << 12) >> 3) + 0]) goto error;
    }

    printf("End clint_vm\n");
    exit(0);

error:
    printf("error: wrong data\n");
    exit(1);
}

int main()
{
    //  initialize the clint
    set_irq_tim_handler(0, clint_timer_handler);
    clint_set_mtimecmp(bsp_get_clint_driver(cpu_id()), 0, TIMER_PERIOD);
    clint_set_mtime(bsp_get_clint_driver(cpu_id()), 0);

    //  initialize and activate virtual memory
    init_vm((uintptr_t)__main);
    return 0;
}
