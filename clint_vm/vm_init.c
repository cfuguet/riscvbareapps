#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <common/compiler.h>

#define PTE_V 0x001
#define PTE_R 0x002
#define PTE_W 0x004
#define PTE_X 0x008
#define PTE_U 0x010
#define PTE_G 0x020
#define PTE_A 0x040
#define PTE_D 0x080

#define PAGE4K_SHIFT 12
#define PAGE2M_SHIFT 21
#define PAGE1G_SHIFT 30
#define PTE_INDEX_MASK 0x1ff

#define addr2vpn4k(x) ((x) >> PAGE4K_SHIFT)
#define addr2ppn4k(x) ((x) >> PAGE4K_SHIFT)

#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT 2
#define PMP_NAPOT 0x18

#define PT1_MAX 16
#define PT2_MAX 16

#ifdef  VM_DEBUG
#define VM_PDEBUG(...) printf("debug: " __VA_ARGS__)
#else
#define VM_PDEBUG(...)
#endif

void init_vm(uintptr_t start_func)
{
    static volatile uint64_t pt0[512] __align(1 << PAGE4K_SHIFT);
    static volatile uint64_t pt1[PT1_MAX][512] __align(1 << PAGE4K_SHIFT);
    static volatile uint64_t pt2[PT2_MAX][512] __align(1 << PAGE4K_SHIFT);
    static int pt1_free;
    static int pt2_free;

    memset((void*)pt0, 0, (1 << PAGE4K_SHIFT));
    pt1_free = 0;
    pt2_free = 0;

    uintptr_t pt1_alloc()
    {
        if (pt1_free > PT1_MAX) {
            puts("error: No level 1 page table available\n");
            exit(1);
        }
        uintptr_t __pt1;
        __pt1 = (uintptr_t)&pt1[pt1_free++][0];
        memset((void*)__pt1, 0, (1 << PAGE4K_SHIFT));
        return __pt1;
    }

    uintptr_t pt2_alloc()
    {
        if (pt2_free > PT2_MAX) {
            puts("error: No level 2 page table available\n");
            exit(1);
        }
        uintptr_t __pt2;
        __pt2 = (uintptr_t)&pt2[pt2_free++][0];
        memset((void*)__pt2, 0, (1 << PAGE4K_SHIFT));
        return __pt2;
    }

    void map_4k_page(uint64_t vpn, uint64_t ppn)
    {
        uintptr_t pt1_base, pt2_base;
        uint64_t  *pt0_ptr, *pt1_ptr, *pt2_ptr;
        uint64_t  pt0_pte, pt1_pte, pt2_pte;
        uintptr_t vpn0, vpn1, vpn2;

        vpn0 = (vpn >> (9*0)) & PTE_INDEX_MASK;
        vpn1 = (vpn >> (9*1)) & PTE_INDEX_MASK;
        vpn2 = (vpn >> (9*2)) & PTE_INDEX_MASK;

        VM_PDEBUG("debug: mapping vpn=%x to ppn=%x\n", vpn, ppn);

        //  Map first level of the page table
        pt0_ptr = (uint64_t*)pt0;
        pt0_pte = pt0[vpn2];
        if ((pt0_pte & PTE_V) == 0) {
            VM_PDEBUG("debug: allocating new pt1\n");
            pt1_base = pt1_alloc();
            pt0_ptr[vpn2] = (addr2ppn4k(pt1_base) << 10) | PTE_V;
        } else {
            VM_PDEBUG("debug: reusing pt1\n");
            pt1_base = (pt0_pte >> 10) << PAGE4K_SHIFT;
        }
        VM_PDEBUG("debug: pt1_base=%x\n", pt1_base);

        //  Map second level of the page table
        pt1_ptr = (uint64_t*)pt1_base;
        pt1_pte = pt1_ptr[vpn1];
        if ((pt1_pte & PTE_V) == 0) {
            VM_PDEBUG("debug: allocating new pt2\n");
            pt2_base = pt2_alloc();
            pt1_ptr[vpn1] = (addr2ppn4k(pt2_base) << 10) | PTE_V;
        } else {
            VM_PDEBUG("debug: reusing pt2\n");
            pt2_base = (pt1_pte >> 10) << PAGE4K_SHIFT;
        }
        VM_PDEBUG("debug: pt2_base=%x\n", pt1_base);

        //  Map third level of the page table
        pt2_ptr = (uint64_t*)pt2_base;
        pt2_pte = pt2_ptr[vpn0];
        if ((pt2_pte & PTE_V) == 0) {
            pt2_ptr[vpn0] = (ppn << 10) | PTE_D | PTE_A | PTE_X | PTE_W | PTE_R | PTE_V;
        } else {
            printf("error: VPN already mapped\n");
            exit(1);
        }

        VM_PDEBUG("debug: mapping completed\n");
    }

    //  map the RAM segment
    for (int i = 0; i < (0x800000 >> 12); i++) {
        uintptr_t offset = (uintptr_t)i << 12;
        map_4k_page(addr2vpn4k((uintptr_t)0x80000000 + offset),
                    addr2ppn4k((uintptr_t)0x80000000 + offset));
    }

    //  map the UART
    map_4k_page(addr2vpn4k(0x10000000), addr2ppn4k(0x10000000));

    //  map the CLINT
    for (int i = 0; i < (0xc000 >> 12); i++) {
        uintptr_t offset = (uintptr_t)i << 12;
        map_4k_page(addr2vpn4k(0x02000000 + offset),
                    addr2ppn4k(0x02000000 + offset));
    }

    // Set up PMPs if present, ignoring illegal instruction trap if not.
    uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
    uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
    asm volatile (
        "csrw pmpaddr0, %[pmpaddr]     \n"
        "csrw pmpcfg0,  %[pmpcfg]      \n"
        : /* no output */
        : [pmpcfg]"r"(pmpc), [pmpaddr]"r"(pmpa)
        : "memory"
    );

    //  Setup the SATP register and switch to the supervisor privilege level
    uintptr_t pt0_base = (uintptr_t)&pt0[0];
    uintptr_t epc = start_func;
    uint64_t mode = 8; // sv39
    uint64_t asid = 0;

    asm volatile (
        "csrs  mstatus, %[mstatus]      \n"
        "csrw  satp, %[satp]            \n"
        "csrw  mepc, %[mepc]            \n"
        "fence.i                        \n"
        "mret                           \n"
        : // no outputs
        : [mstatus]"r"(1 << 11), // set the SPP bit to 1 to return as supervisor
          [satp]"r"((mode << 60) | (asid << 44) | (pt0_base >> 12)),
          [mepc]"r"(epc)
        : "memory"
    );
}
