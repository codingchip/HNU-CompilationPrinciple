/* pl0.h - PL/0 ������ͷ�ļ� */

#ifndef PL0_H
#define PL0_H

#include <stdio.h>

/* �������� */
#define TXMAX    100     /* ���ű�������� */
#define CXMAX    500     /* ��������������� */
#define STACKSIZE 500    /* ����ʱ����ջ��С */
#define LEVMAX   3       /* ���Ƕ�ײ��� */
#define AMAX     2047    /* ��ַ�Ͻ� */
#define NMAX     14      /* ���ֵ����λ�� */
#define AL       10      /* ��ʶ������󳤶� */

/* �������� */
enum object {
    CONSTANT,
    VARIABLE,
    PROCEDURE_SYM
};

/* �����ָ�� */
enum fct {
    LIT,    /* 0: �������ŵ�ջ�� */
    OPR,    /* 1: ִ������ */
    LOD,    /* 2: �������ŵ�ջ�� */
    STO,    /* 3: ��ջ�����ݴ浽���� */
    CAL,    /* 4: ���ù��� */
    INT,    /* 5: ����ռ� */
    JMP,    /* 6: ��������ת */
    JPC     /* 7: ������ת */
};

/* ���ű�ṹ */
struct symbol {
    char name[AL + 1];
    enum object kind;
    int val;    /* CONSTANTʹ�� */
    int level;  /* ���ڲ�� */
    int adr;    /* ��ַ */
    int size;   /* PROCEDUREʹ�� */
};

/* ָ��ṹ */
struct instruction {
    enum fct f;  /* ������ */
    int l;       /* ��� */
    int a;       /* λ�ƻ������ */
};

/* ȫ�ֱ������� */
extern char id[AL + 1];  /* ��ǰ��ʶ�� */
extern int num;          /* ��ǰ���� */
extern int cx;           /* ����������� */
extern int level;        /* ��ǰ��� */
extern int tx;           /* ���ű�ǰβָ�� */
extern int dx;           /* ���ݷ������� */
extern int err_count;    /* ������� */

extern struct instruction code[CXMAX];  /* ������������ */
extern struct symbol table[TXMAX];      /* ���ű� */

/* ������ */
void error(int n);

/* ���ű���� */
void enter(enum object kind);
int position(char *id);

/* �������� */
void gen(enum fct f, int l, int a);
void listcode(int from, int to);

/* ����� */
void interpret();
int base(int l, int b, int s[]);

#endif /* PL0_H */
