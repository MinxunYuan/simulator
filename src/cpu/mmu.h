// manage management unit
#ifndef mmu_gard

#define mmu_guard
#include <stdint.h>
// 物理地址-虚拟地址
uint64_t va2pa(uint64_t vaddr);

#endif
// singleton