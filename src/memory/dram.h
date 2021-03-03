#ifndef dram_guard
#define dram_guard

#include <stdint.h>
#define MM_LEN 1000

uint8_t mm[MM_LEN]; // physical addr
// 拿过来的是虚拟地址，要先翻译成物理地址，要么塞不下

// virtual addr = 0x0~0xffffffffffffffff(0 ~ 2^52 - 1)
// physical addr = 000~999

uint64_t read64bits_dram(uint64_t paddr);
void write64bits_dram(uint64_t paddr, uint64_t data);
void print_register();
void print_stack();

#endif