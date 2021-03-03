#include <stdio.h>
#include <stdint.h>

#include "memory/dram.h"
#include "cpu/register.h"
#include "cpu/mmu.h"
#include "memory/instruction.h"
#include "memory/dram.h"
#include "disk/elf.h"

// 简单地mention一下条件include
/* 编译器 */

int main()
{

    // 初始话函数指针数组handler_t*
    init_handler_table();

    // 比较初始状态reg.rax和reg.rbx
    reg.rax = 0x12340000;
    reg.rbx = 0x0;
    reg.rcx = 0x8000660;
    reg.rdx = 0xabcd;
    reg.rsi = 0x7ffffffee2f8;
    reg.rdi = 0x1;
    reg.rbp = 0x7ffffffee210;
    reg.rsp = 0x7ffffffee1f0;

    // rip从program[11] mov %rdx,%rsi
    // program[11]类型是inst_t, 要转成uint64_t的地址
    // 然后是mov %rax,%rdi 和 call addr
    reg.rip = (uint64_t)&program[11];

    write64bits_dram(va2pa(0x7ffffffee210), 0x8000660);
    write64bits_dram(va2pa(0x7ffffffee208), 0x0);
    write64bits_dram(va2pa(0x7ffffffee200), 0xabcd);
    write64bits_dram(va2pa(0x7ffffffee1f8), 0x12340000);
    write64bits_dram(va2pa(0x7ffffffee1f0), 0x8000660);

    // uint64_t pa = va2pa(0x7ffffffee210);
    // printf("%16lx\n", *((uint64_t *)(&mm[pa])));
    // printf("%lx\n", read64bits_dram(va2pa(0x7ffffffee210)));
    // mem(模拟器的mm数组是 uint8_t mm[MM_LEN]，所以说记得cast一下)
    // 其实可以encapsulate一下，根据是否cache miss来看要不要去DRAM

    print_register();
    print_stack();

    // run
    for (int i = 0; i < 15; i++)
    {
        printf("%d\n", i);
        // 暂时先decode 15条inst
        instruction_cycle();
        print_register();
        print_stack();
    }

    // verify
    int match = 1;

    match = match && (reg.rax == 0x1234abcd);
    match = match && (reg.rbx == 0x0);
    match = match && (reg.rcx == 0x8000660);
    match = match && (reg.rdx == 0x12340000);
    match = match && (reg.rsi == 0xabcd);
    match = match && (reg.rdi == 0x12340000);
    match = match && (reg.rbp == 0x7ffffffee210);
    match = match && (reg.rsp == 0x7ffffffee1f0);

    if (match)
    {
        printf("register match\n");
    }
    else
    {
        printf("register not match\n");
    }

    // check内存的话看5个uint_64就ok了（从rbp~rsp)
    match = match && (read64bits_dram(va2pa(0x7ffffffee210)) == 0x08000660);
    match = match && (read64bits_dram(va2pa(0x7ffffffee208)) == 0x1234abcd);
    match = match && (read64bits_dram(va2pa(0x7ffffffee200)) == 0xabcd);
    match = match && (read64bits_dram(va2pa(0x7ffffffee1f8)) == 0x12340000);
    match = match && (read64bits_dram(va2pa(0x7ffffffee1f0)) == 0x08000660);

    if (match)
    {
        printf("memory match\n");
    }
    else
    {
        printf("memory not match\n");
    }
    return 0;
}
