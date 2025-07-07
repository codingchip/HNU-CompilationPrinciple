/* pl0.y - PL/0 语法分析器 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pl0.h"

extern int yylex();
extern void yyerror(const char *s);
extern int line_no;
extern int col_no;
extern FILE *yyin;

/* 全局变量 */
int cx = 0;                     /* 代码索引 */
int level = 0;                  /* 当前层次 */
int tx = 0;                     /* 符号表索引 */
int dx = 0;                     /* 数据分配索引 */
int err_count = 0;              /* 错误计数 */

struct instruction code[CXMAX]; /* 代码数组 */
struct symbol table[TXMAX];     /* 符号表 */

/* 函数声明 */
void enter(enum object kind);
int position(char *id);
void gen(enum fct f, int l, int a);
void error(int n);
void listcode(int from, int to);
void interpret();

%}

%union {
    int number;
    char *ident;
}

%token <number> NUMBER
%token <ident> IDENT
%token CONST VAR PROCEDURE CALL BEGIN_SYM END IF THEN WHILE DO ODD READ WRITE
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES SLASH
%token LPAREN RPAREN COMMA SEMICOLON PERIOD

%left PLUS MINUS
%left TIMES SLASH
%nonassoc UMINUS

%%

program:
    block PERIOD
    {
        gen(OPR, 0, 0);  /* 返回指令 */
        if (err_count == 0) {
            printf("\nCompilation successful!\n");
            listcode(0, cx);
            printf("\nStart PL/0\n");
            interpret();
        } else {
            printf("\n%d errors in PL/0 program\n", err_count);
        }
    }
    ;

block:
    {
        dx = 3;           /* 为链接数据预留空间 */
        int jmpaddr = cx;
        gen(JMP, 0, 0);   /* 产生跳转指令，跳转地址未知 */
        
        if (level > LEVMAX) {
            error(32);    /* 嵌套层次过深 */
        }
        $<number>$ = jmpaddr;
    }
    declaration_list
    {
        int jmpaddr = $<number>1;
        code[jmpaddr].a = cx;    /* 回填跳转地址 */
        /* 如果是过程，记录其入口地址 */
        int i;
        for (i = tx; i > 0; i--) {
            if (table[i].kind == PROCEDURE_SYM && table[i].adr == 0) {
                table[i].adr = cx;
                table[i].size = dx;
                break;
            }
        }
        gen(INT, 0, dx);         /* 分配空间 */
    }
    statement
    ;

declaration_list:
    /* empty */
    | declaration_list declaration
    ;

declaration:
    const_declaration
    | var_declaration
    | proc_declaration
    ;

const_declaration:
    CONST const_list SEMICOLON
    ;

const_list:
    const_def
    | const_list COMMA const_def
    ;

const_def:
    IDENT EQ NUMBER
    {
        strcpy(id, $1);
        num = $3;
        enter(CONSTANT);
        free($1);
    }
    ;

var_declaration:
    VAR var_list SEMICOLON
    ;

var_list:
    IDENT
    {
        strcpy(id, $1);
        enter(VARIABLE);
        free($1);
    }
    | var_list COMMA IDENT
    {
        strcpy(id, $3);
        enter(VARIABLE);
        free($3);
    }
    ;

proc_declaration:
    PROCEDURE IDENT
    {
        strcpy(id, $2);
        enter(PROCEDURE_SYM);
        free($2);
        level++;
        dx = 3;  /* 重置数据分配索引 */
    }
    SEMICOLON block SEMICOLON
    {
        gen(OPR, 0, 0);  /* 过程返回 */
        level--;
        /* 不重置tx，保留所有符号在符号表中 */
    }
    ;

statement:
    /* empty */
    | assignment_statement
    | call_statement
    | compound_statement
    | if_statement
    | while_statement
    | read_statement
    | write_statement
    ;

assignment_statement:
    IDENT ASSIGN expression
    {
        int i = position($1);
        if (i == 0) {
            error(11);  /* 标识符未声明 */
        } else if (table[i].kind != VARIABLE) {
            error(12);  /* 不能给常量或过程赋值 */
        } else {
            gen(STO, level - table[i].level, table[i].adr);
        }
        free($1);
    }
    ;

call_statement:
    CALL IDENT
    {
        int i = position($2);
        if (i == 0) {
            error(11);  /* 标识符未声明 */
        } else if (table[i].kind != PROCEDURE_SYM) {
            error(15);  /* 调用非过程标识符 */
        } else {
            gen(CAL, level - table[i].level, table[i].adr);
        }
        free($2);
    }
    ;

compound_statement:
    BEGIN_SYM statement_list END
    ;

statement_list:
    statement
    | statement_list SEMICOLON statement
    ;

if_statement:
    IF condition THEN statement
    {
        code[$<number>2].a = cx;  /* 回填跳转地址 */
    }
    ;

while_statement:
    WHILE
    {
        $<number>$ = cx;  /* 保存循环开始地址 */
    }
    condition DO
    {
        $<number>$ = $<number>3;  /* 保存JPC地址 */
    }
    statement
    {
        gen(JMP, 0, $<number>2);      /* 跳回循环开始 */
        code[$<number>5].a = cx;      /* 回填条件跳转地址 */
    }
    ;
    
read_statement:
    READ LPAREN read_list RPAREN
    ;

read_list:
    IDENT
    {
        int i = position($1);
        if (i == 0) {
            error(11);  /* 标识符未声明 */
        } else if (table[i].kind != VARIABLE) {
            error(12);  /* 只能读入变量 */
        } else {
            gen(OPR, 0, 16);  /* 读操作 */
            gen(STO, level - table[i].level, table[i].adr);
        }
        free($1);
    }
    | read_list COMMA IDENT
    {
        int i = position($3);
        if (i == 0) {
            error(11);
        } else if (table[i].kind != VARIABLE) {
            error(12);
        } else {
            gen(OPR, 0, 16);
            gen(STO, level - table[i].level, table[i].adr);
        }
        free($3);
    }
    ;

write_statement:
    WRITE LPAREN write_list RPAREN
    {
        gen(OPR, 0, 15);  /* 输出换行 */
    }
    ;

write_list:
    expression
    {
        gen(OPR, 0, 14);  /* 输出栈顶值 */
    }
    | write_list COMMA expression
    {
        gen(OPR, 0, 14);
    }
    ;

condition:
    ODD expression
    {
        gen(OPR, 0, 6);  /* ODD操作 */
        $<number>$ = cx;
        gen(JPC, 0, 0);  /* 条件跳转 */
    }
    | expression rel_op expression
    {
        gen(OPR, 0, $<number>2);  /* 关系运算 */
        $<number>$ = cx;
        gen(JPC, 0, 0);           /* 条件跳转 */
    }
    ;

rel_op:
    EQ    { $<number>$ = 8; }
    | NE  { $<number>$ = 9; }
    | LT  { $<number>$ = 10; }
    | GE  { $<number>$ = 11; }
    | GT  { $<number>$ = 12; }
    | LE  { $<number>$ = 13; }
    ;

expression:
    term
    | PLUS term
    | MINUS term %prec UMINUS
    {
        gen(OPR, 0, 1);  /* 取负 */
    }
    | expression PLUS term
    {
        gen(OPR, 0, 2);  /* 加法 */
    }
    | expression MINUS term
    {
        gen(OPR, 0, 3);  /* 减法 */
    }
    ;

term:
    factor
    | term TIMES factor
    {
        gen(OPR, 0, 4);  /* 乘法 */
    }
    | term SLASH factor
    {
        gen(OPR, 0, 5);  /* 除法 */
    }
    ;

factor:
    IDENT
    {
        int i = position($1);
        if (i == 0) {
            error(11);  /* 标识符未声明 */
        } else {
            switch (table[i].kind) {
                case CONSTANT:
                    gen(LIT, 0, table[i].val);
                    break;
                case VARIABLE:
                    gen(LOD, level - table[i].level, table[i].adr);
                    break;
                case PROCEDURE_SYM:
                    error(21);  /* 表达式中不能有过程标识符 */
                    break;
            }
        }
        free($1);
    }
    | NUMBER
    {
        if ($1 > AMAX) {
            error(30);  /* 数值越界 */
            $1 = 0;
        }
        gen(LIT, 0, $1);
    }
    | LPAREN expression RPAREN
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, line_no, col_no);
    err_count++;
}