#ifndef inst_guard
#define inst_guard

#include <stdlib.h>
#include <stdint.h>

#define NUM_INSTRTYPE 30 // op一共有多少种？

#define INST_LEN 100

// 操作命令，每种指令对应一个Handler函数
typedef enum OP
{
    // MOV,   0 move指令有很多条，可以细化(到时候再整)
    mov_reg_reg,
    mov_reg_mem,
    mov_mem_reg,
    push_reg,
    pop_reg,
    call,
    ret,
    add_reg_reg
    // 暂时先用8个其实还有很多
} op_t;

// 3.4.1 Operand Specifiers(指令的操作数由什么组成？分11种 enum)
typedef enum OD_TYPE
{
    EMPTY, // 这个坑位占operand
    IMM,
    REG,
    MM_IMM,
    MM_REG,
    MM_IMM_REG,
    MM_REG1_REG2,
    MM_IMM_REG1_REG2,
    MM_REG2_S,
    MM_IMM_REG2_S,
    MM_REG1_REG2_S,
    MM_IMM_REG1_REG2_S
} od_type_t;

/* 
一个抽象的operand可能consists of的东西
比如说mov %rdi,-0x18(%rbp)这条指令中的-0x18(%rbp)首先这个operator设计memory
然后它包含了一个立即数-0x18和一个放地址的寄存器%rbp
就是说：你要去访问某个addr的memory，这个地址在哪呢？%rbp寄存器放一个地址
然后-0x18为displacement，所以说你要去的地址就是-0x18+(addr in %rpb)

比如说mov %rsp,%rpb这种就是纯粹把rsp寄存器里面的东西放到rpb里面，不管里面是什么
那这条inst的两个operand都是register了，
%rsp构造出来的(匿名)结构体就是
{REG,0,0,reg.rsp的地址转成指向uint64_t的指针uint64_t *, NULL}
（原因到时候再说）
*/
typedef struct OD
{
    od_type_t type;

    int64_t imm; // 立即数
    int64_t scal;
    uint64_t *reg1;
    uint64_t *reg2;
} od_t;

typedef struct INSTRUCT_STRUCT
{
    op_t op; // mov, push, call..
    // 两个operands
    od_t src;
    od_t dst;
    char code[100]; // 自己看的
} inst_t;

inst_t program[INST_LEN];

uint64_t decode_od(od_t od);

typedef void (*handler_t)(uint64_t, uint64_t);
// 定义了个handler_t类型的函数，参数都是uint64_t

// 函数指针数组（到时候要初始化）
handler_t handler_table[NUM_INSTRTYPE];

// 初始化指令集（表）
void init_handler_table();

void instruction_cycle();

// 每一种类型的指令对应的handler（decode之后把reg的有效成员丢进来）
void add_reg_reg_handler(uint64_t src, uint64_t dst);
void call_handler(uint64_t src, uint64_t dst);
void mov_reg_reg_handler(uint64_t src, uint64_t dst);
void push_reg_handler(uint64_t src, uint64_t dst);
void pop_reg_handler(uint64_t src, uint64_t dst);
void mov_reg_mem_handler(uint64_t src, uint64_t dst);
void mov_mem_reg_handler(uint64_t src, uint64_t dst);
void ret_handler(uint64_t src, uint64_t dst);

#endif