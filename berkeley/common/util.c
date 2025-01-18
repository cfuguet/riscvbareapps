#include "util.h"
#include "common/cpu.h"
#include <stdio.h>

#define NUM_COUNTERS 2
static uintptr_t counters[NUM_COUNTERS];
static char* counter_names[NUM_COUNTERS];

void setStats(int enable)
{
  int i = 0;
#define READ_CTR(name) do { \
    while (i >= NUM_COUNTERS) ; \
    uintptr_t csr = read_csr(name); \
    if (!enable) { csr -= counters[i]; counter_names[i] = #name; } \
    counters[i++] = csr; \
  } while (0)

  READ_CTR(mcycle);
  READ_CTR(minstret);

#undef READ_CTR
}

void printStats()
{
  printf("%s = %lu", counter_names[0], counters[0]);
  for (int i = 1; i < NUM_COUNTERS; i++) {
    printf(", %s = %lu", counter_names[i], counters[i]);
  }
}
