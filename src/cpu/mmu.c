#include "cpu/mmu.h"
#include "memory/dram.h"

uint64_t va2pa(uint64_t vaddr)
{
    // 假的虚拟地址-物理地址的mapping（到virtual Memory的时候再改）
    return vaddr % MM_LEN;
}