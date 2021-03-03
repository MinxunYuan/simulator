#include "memory/instruction.h"
#include "cpu/mmu.h"
#include "cpu/register.h"
#include "memory/dram.h"

#include <stdio.h>
uint64_t decode_od(od_t od)
{
    // 反正就3个type的operand
    if (od.type == IMM)
    {
        return *((uint64_t *)&od.imm);
    }
    else if (od.type == REG)
    {
        /* 
         */
        return (uint64_t)od.reg1;
    }
    else
    {
        /* 拿virtual memory
        如果reg.rsp 在0xabcd上，值是1，模拟器上是pmm[0xabcd] = 0x1
        得拿到0x1的地址，也就是0xabcd，相当于是模拟器的内存地址是index
        pmm[addr]是动态变化的
        */
        uint64_t vaddr = 0;
        // 事先规定一下，书上的有ra absolute，rb base，ri indirect
        // 但是之前做的od结构体里面只有 *reg1和*reg2
        // 当M[R[ra]]的时候，r1当做ra，当M[R[Imm+rb]]的时候r1当做rb
        if (od.type == MM_IMM)
        {
            // MM_IMM对应书上的指令是M[Imm] Absolute
            vaddr = od.imm;
        }
        else if (od.type == MM_REG)
        {
            // M[R[ra]]给了地址，indirect解引用拿值
            vaddr = *(od.reg1);
        }
        else if (od.type == MM_IMM_REG)
        {
            // M[Imm+R[rb]] base+displacement
            // 地址为rb寄存器里面的值+个立即数
            vaddr = od.imm + *(od.reg1);
        }
        else if (od.type == MM_REG1_REG2)
        {
            // M[Imm+R[rb]]:Indexed
            vaddr = *(od.reg1) + *(od.reg2);
        }
        else if (od.type == MM_IMM_REG1_REG2)
        {
            // 类似访问数组 indexed
            vaddr = *(od.reg1) + *(od.reg2) + od.imm;
        }
        else if (od.type == MM_REG2_S)
        {
            // scaled index: M[R[ri]*s]，先把地址拿出来，再*scale
            vaddr = (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_IMM_REG2_S)
        {
            // M[Imm + R[ri]*s]上面那个再+立即数
            vaddr = od.imm + (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_REG1_REG2_S)
        {
            /* 
            M[R[rb]+R[ri].s]: 
            可以理解为上面那个版本，但是立即数放在rb寄存器里面，R[rb]
            所以说得先indirect一下
            */
            vaddr = *(od.reg1) + (*(od.reg2)) * od.scal;
        }
        else if (od.type == MM_IMM_REG1_REG2_S)
        {
            // M[Imm + R[rb] + R[ri].s]:取Imm + R[rb] + R[ri].s这一块地址
            vaddr = od.imm + *(od.reg1) + (*(od.reg2)) * od.scal;
        }
        return vaddr;
    }
}

// exe current inst & PC++
void instruction_cycle()
{
    inst_t *instr = (inst_t *)reg.rip;
    // imm:imm
    // reg:value
    // mm（9种形式）得到物理地址（其实是模拟器的uint8_t mm[MM_LEN]的index）
    uint64_t src = decode_od(instr->src);
    uint64_t dst = decode_od(instr->dst);

    // add rax rbx
    // op = add_reg_reg = 3
    // handler_table[add_reg_reg] == handler_table[3] = add_reg_reg_handler

    // 每一个元素都是一个函数指针
    handler_t handler = handler_table[instr->op]; // add_reg_reg_handler
    handler(src, dst);
    printf("    %s\n", instr->code);

    // 函数数组，每一个元素都是一个函数
    // add_reg_reg_handle(src = &rax, dst = &rbx)
    /* 
    rax pmm[0x1234] = 0x12340000 
    rbx pmm[0x1235] = 0xabcd
    src: 0x1234
    dst: 0x1235
    *(uint64_t *)src = 0x12340000
    *(uint64_t *)src = 0xabcd
    0x12340000 + 0xabcd = 0x1234abcd
    执行了add rax abx后
    pmm[0x1235] = 0x1234abcd


    */
    // 每一个汇编服从格式：只需要知道operand，两个operator（源操作数，目的操作数）
}

void init_handler_table()
{
    // 初始化handler table(type->function pointer)
    // 其实就是枚举当table的index，找到函数地址
    handler_table[mov_reg_reg] = &mov_reg_reg_handler;
    handler_table[call] = &call_handler;
    handler_table[add_reg_reg] = &add_reg_reg_handler;
    handler_table[push_reg] = &push_reg_handler;
    handler_table[pop_reg] = &pop_reg_handler;
    handler_table[mov_reg_mem] = &mov_reg_mem_handler;
    handler_table[mov_mem_reg] = &mov_mem_reg_handler;
    handler_table[ret] = &ret_handler;
}

// push what holds on src register onto stack
void push_reg_handler(uint64_t src, uint64_t dst)
{
    // src: reg
    // dst: empty
    reg.rsp = reg.rsp - 0x8;
    write64bits_dram(va2pa(reg.rsp), *(uint64_t *)src);
    reg.rip = reg.rip + sizeof(inst_t);
}

void pop_reg_handler(uint64_t src, uint64_t dst)
{
    // src: reg（还是解释成一个qword的指针，然后再deference一下）
    *(uint64_t *)src = read64bits_dram(va2pa(reg.rsp));
    reg.rsp += 8;
    reg.rip += sizeof(inst_t);
}

// 恢复一下caller frame的%rpb
// 因为call的时候是有%push rpb保护现场,然后把caller的栈顶当做callee的栈底
// 返回的话那就rbp—>rsp然后mov -8(rsp) rbp;
// 然后rsp下面的其实不用手动清零，到时候会被别的过程覆盖掉的（别用它就行了）
void ret_handler(uint64_t src, uint64_t dst)
{
    // TODO
    uint64_t ret_addr = read64bits_dram(va2pa(reg.rsp));
    reg.rip = ret_addr;
    reg.rsp += 8;
}

// 把src寄存器值放到dst地址的内存单元中
void mov_reg_mem_handler(uint64_t src, uint64_t dst)
{
    // src: reg
    // reg: vmem
    // *(uint64_t *)dst = *(uint64_t *)src;
    write64bits_dram(va2pa(dst), *(uint64_t *)src);
    reg.rip = reg.rip + sizeof(inst_t);
}

// 内存单元数据move到寄存器上(直接从内存读)
void mov_mem_reg_handler(uint64_t src, uint64_t dst)
{
    // src: mem vaddr
    // dst: reg
    *(uint64_t *)dst = read64bits_dram(va2pa(src));
    reg.rip = reg.rip + sizeof(inst_t);
}

void call_handler(uint64_t src, uint64_t dst)
{
    // src: imm add of called func (callq 400540 <leaf>)
    reg.rsp = reg.rsp - 8;
    // write return address to rsp（把过程调用的下一句话给push入栈）
    // pc是当前call指令，pc++是call下面，也就是子过程结束之后将要执行的那句话
    write64bits_dram(va2pa(reg.rsp), reg.rip + sizeof(inst_t));
    // 然后再去从源操作数里面把子过程入口地址拿到
    reg.rip = src;
}
void mov_reg_reg_handler(uint64_t src, uint64_t dst)
{
    // 就是把src寄存器里里面的放到dst的 然后把PC++

    *(uint64_t *)dst = *(uint64_t *)src;
    reg.rip = reg.rip + sizeof(inst_t);
}

void add_reg_reg_handler(uint64_t src, uint64_t dst)
{
    // src和dst是寄存器uint64_t rax;的地址（其实不是这样的，但是模拟器是的）
    // 那*(uint64_t *)dst就是rax的值
    *(uint64_t *)dst = *(uint64_t *)dst + *(uint64_t *)src;
    // pc++
    reg.rip = reg.rip + sizeof(inst_t);
}
