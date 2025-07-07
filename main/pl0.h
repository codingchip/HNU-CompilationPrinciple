/* pl0.h - PL/0 编译器头文件 */

#ifndef PL0_H
#define PL0_H

#include <stdio.h>

/* 常量定义 */
#define TXMAX    100     /* 符号表最大容量 */
#define CXMAX    500     /* 最多的虚拟机代码数 */
#define STACKSIZE 500    /* 运行时数据栈大小 */
#define LEVMAX   3       /* 最大嵌套层数 */
#define AMAX     2047    /* 地址上界 */
#define NMAX     14      /* 数字的最大位数 */
#define AL       10      /* 标识符的最大长度 */

/* 符号类型 */
enum object {
    CONSTANT,
    VARIABLE,
    PROCEDURE_SYM
};

/* 虚拟机指令 */
enum fct {
    LIT,    /* 0: 将常数放到栈顶 */
    OPR,    /* 1: 执行运算 */
    LOD,    /* 2: 将变量放到栈顶 */
    STO,    /* 3: 将栈顶内容存到变量 */
    CAL,    /* 4: 调用过程 */
    INT,    /* 5: 分配空间 */
    JMP,    /* 6: 无条件跳转 */
    JPC     /* 7: 条件跳转 */
};

/* 符号表结构 */
struct symbol {
    char name[AL + 1];
    enum object kind;
    int val;    /* CONSTANT使用 */
    int level;  /* 所在层次 */
    int adr;    /* 地址 */
    int size;   /* PROCEDURE使用 */
};

/* 指令结构 */
struct instruction {
    enum fct f;  /* 功能码 */
    int l;       /* 层差 */
    int a;       /* 位移或运算号 */
};

/* 全局变量声明 */
extern char id[AL + 1];  /* 当前标识符 */
extern int num;          /* 当前数字 */
extern int cx;           /* 代码分配索引 */
extern int level;        /* 当前层次 */
extern int tx;           /* 符号表当前尾指针 */
extern int dx;           /* 数据分配索引 */
extern int err_count;    /* 错误计数 */

extern struct instruction code[CXMAX];  /* 存放虚拟机代码 */
extern struct symbol table[TXMAX];      /* 符号表 */

/* 错误处理 */
void error(int n);

/* 符号表管理 */
void enter(enum object kind);
int position(char *id);

/* 代码生成 */
void gen(enum fct f, int l, int a);
void listcode(int from, int to);

/* 虚拟机 */
void interpret();
int base(int l, int b, int s[]);

#endif /* PL0_H */
