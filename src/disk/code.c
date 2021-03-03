#include <stdlib.h>

#include "memory/instruction.h"
#include "cpu/register.h"
// 人肉decode
inst_t program[INST_LEN] =
    {
        // push %rbp
        // 把当前栈底位置压入栈（子过程结束的时候得知道父过程在哪开始的）
        {push_reg,
         // 匿名的uint64_t *reg1，先取reg.rbp的地址才能解释为匿名的uint64_t*指针
         {REG, 0, 0, (uint64_t *)&reg.rbp, NULL},
         //  没有目标操作数
         {EMPTY, 0, 0, NULL, NULL},
         "push \%rbp"},
        // 弄的(uint64_t *)&reg.rbp是register.h里面reg_t结构体对象reg中成员的地址
        // reg_t reg;是在stack上面的
        {/*
        mov %rsp,%rbp 现在的栈底变成原来栈顶（此时%rsp和%rbp是一样的）
        准备%rsp--栈从高地址往低地址扩
        */
         mov_reg_reg,
         {REG, 0, 0, (uint64_t *)&reg.rsp, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rbp, NULL},
         "mov    \%rsp,\%rbp"},
        {/* 
            这个是要fetch from memory的（前缀MM），%rbp拿着内存地址
            move指令：mov %rdi,-0x18(%rbp) 
            把rdi寄存器里面的东西放到(addr inside rpb)-0x18这个add中
            */
         mov_reg_mem,
         {REG, 0, 0, (uint64_t *)&reg.rdi, NULL},
         {MM_IMM_REG, -0x18, 0, (uint64_t *)&reg.rbp, NULL},
         "mov   \%rdi,-18(\%rbp)"},
        {mov_reg_mem,
         {REG, 0, 0, (uint64_t *)&reg.rsi, NULL},
         {MM_IMM_REG, -0x20, 0, (uint64_t *)&reg.rbp, NULL},
         "mov   \%rsi,-0x20(\%rbp)"},
        //  mar2
        {mov_mem_reg,
         {MM_IMM_REG, -0x18, 0, (uint64_t *)&reg.rbp, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rdx, NULL},
         "mov    -0x18(\%rbp),%rdx"},
        {mov_mem_reg,
         {MM_IMM_REG, -0x20, 0, (uint64_t *)&reg.rbp, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         "mov    -0x20(\%rpb),%rax"},
        {add_reg_reg,
         {REG, 0, 0, (uint64_t *)&reg.rdx, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         "add   \%rdx,\%rax"},
        {mov_reg_mem,
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         {MM_IMM_REG, -0x8, 0, (uint64_t *)&reg.rbp, NULL},
         "mov   %rax,-0x8(\%rpb)"},
        {mov_mem_reg,
         {MM_IMM_REG, -0x8, 0, (uint64_t *)&reg.rbp, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         "mov   -0x8(\%rbp),\%rax"},
        {pop_reg,
         {REG, 0, 0, (uint64_t *)&reg.rbp, NULL},
         {EMPTY, 0, 0, NULL, NULL},
         "pop   \%rbp"},
        {ret,
         {EMPTY, 0, 0, NULL, NULL},
         {EMPTY, 0, 0, NULL, NULL},
         "retq"},
        {mov_reg_reg,
         {REG, 0, 0, &reg.rdx, NULL},
         {REG, 0, 0, &reg.rsi, NULL},
         "mov   \%rdx,%rsi"},
        {mov_reg_reg,
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         {REG, 0, 0, (uint64_t *)&reg.rdi, NULL},
         "mov   \%rax,\%rdi"},
        {call, // call一个立即数的地址，procedure的入口，push %rbp给的
         {IMM, (uint64_t) & (program[0]), 0, NULL, NULL},
         {EMPTY, 0, 0, NULL, NULL},
         "call  <add>"},
        {mov_reg_mem,
         {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
         {MM_IMM_REG, -0x8, 0, (uint64_t *)&reg.rbp, NULL},
         "mov     \%rax,-0x8(\%rbp)"}
        // {MOV,
        //  {REG, 0, 0, (uint64_t *)&reg.rax, NULL},
        //  {REG, 0, 0, (uint64_t *)&reg.rbx, NULL},
        //  "mov rax rbx"},
};