/* pl0.c - PL/0 ������������ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pl0.h"
#include "pl0.tab.h"

/* ȫ�ֱ������� */
char id[AL + 1];
int num;
struct instruction code[CXMAX];
struct symbol table[TXMAX];
char mnemonic[8][5] = {
    "LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"
};

/* flex������� */
extern FILE *yyin;
extern int yyparse();

/* ������Ϣ�� */
char *err_msg[] = {
    "",  /* 0 */
    "Found '=' when expecting ':='.",
    "There must be a number to follow '='.",
    "There must be an '=' to follow the identifier.",
    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
    "Missing ',' or ';'.",  /* 5 */
    "Incorrect procedure name.",
    "Statement expected.",
    "Follow the statement is an incorrect symbol.",
    "Period expected.",
    "Semicolon between statements is missing.",  /* 10 */
    "Undeclared identifier.",
    "Illegal assignment.",
    "':=' expected.",
    "There must be an identifier to follow the 'call'.",
    "A constant or variable can not be called.",  /* 15 */
    "'then' expected.",
    "Semicolon or 'end' expected.",
    "'do' expected.",
    "Incorrect symbol.",
    "Relative operators expected.",  /* 20 */
    "Procedure identifier can not be in an expression.",
    "Missing ')'.",
    "The symbol can not be followed by a factor.",
    "The symbol can not be as the beginning of an expression.",
    "The number is too great.",  /* 25 */
    "",
    "",
    "",
    "",
    "The number is too great.",  /* 30 */
    "",
    "There are too many levels."
};

/* �������� */
void error(int n) {
    printf("Error %d: %s\n", n, err_msg[n]);
    err_count++;
}

/* �ڷ��ű��еǼǷ��� */
void enter(enum object kind) {
    tx++;
    if (tx > TXMAX) {
        printf("Program too long\n");
        exit(1);
    }
    
    strcpy(table[tx].name, id);
    table[tx].kind = kind;
    
    switch (kind) {
        case CONSTANT:
            if (num > AMAX) {
                error(30);
                num = 0;
            }
            table[tx].val = num;
            break;
            
        case VARIABLE:
            table[tx].level = level;
            table[tx].adr = dx++;
            break;
            
        case PROCEDURE_SYM:
            table[tx].level = level;
            break;
    }
}

/* ���ұ�ʶ���ڷ��ű��е�λ�� */
int position(char *id) {
    int i;
    strcpy(table[0].name, id);  /* �ڱ� */
    i = tx;
    
    /* �ӵ�ǰλ����ǰ������֧��Ƕ�������� */
    while (strcmp(table[i].name, id) != 0) {
        i--;
    }
    
    /* ����ҵ������ڱ���i==0����˵��δ�ҵ� */
    if (i == 0) {
        return 0;
    }
    
    /* ����ҵ��ı�ʶ���Ƿ��ڵ�ǰ�ɷ��ʵ��������� */
    /* PL/0 �����ڲ�������ı�ʶ�� */
    if (table[i].level <= level) {
        return i;
    }
    
    /* ����ҵ��ı�ʶ���ڸ���Ĳ�Σ�������ǰ���� */
    int saved_i = i;
    i--;
    while (i > 0 && strcmp(table[i].name, id) != 0) {
        i--;
    }
    
    if (i > 0 && table[i].level <= level) {
        return i;
    }
    
    /* ���û���ҵ����ʵģ����ص�һ���ҵ��ģ����ܱ��� */
    return saved_i;
}

/* ���������ָ�� */
void gen(enum fct f, int l, int a) {
    if (cx >= CXMAX) {
        printf("Program too long\n");
        exit(1);
    }
    code[cx].f = f;
    code[cx].l = l;
    code[cx].a = a;
    cx++;
}

/* ��������б� */
void listcode(int from, int to) {
    int i;
    printf("\n=== OBJECT CODE ===\n");
    for (i = from; i < to; i++) {
        printf("%3d: %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
    }
}

/* ͨ����̬������l��Ļ���ַ */
int base(int l, int b, int s[]) {
    int b1 = b;
    while (l > 0) {
        b1 = s[b1];
        l--;
    }
    return b1;
}

/* ���������ִ�� */
void interpret() {
    int p = 0;      /* ��������� */
    int b = 1;      /* ����ַ�Ĵ��� */
    int t = 0;      /* ջ���Ĵ��� */
    int s[STACKSIZE] = {0};  /* ����ջ */
    struct instruction i;    /* ��ǰָ�� */
    
    printf("\n=== RUNNING PL/0 ===\n");
    s[1] = s[2] = s[3] = 0;
    
    do {
        i = code[p++];
        
        switch (i.f) {
            case LIT:
                s[++t] = i.a;
                break;
                
            case OPR:
                switch (i.a) {
                    case 0:  /* ���� */
                        t = b - 1;
                        p = s[t + 3];
                        b = s[t + 2];
                        break;
                    case 1:  /* ȡ�� */
                        s[t] = -s[t];
                        break;
                    case 2:  /* �ӷ� */
                        t--;
                        s[t] = s[t] + s[t + 1];
                        break;
                    case 3:  /* ���� */
                        t--;
                        s[t] = s[t] - s[t + 1];
                        break;
                    case 4:  /* �˷� */
                        t--;
                        s[t] = s[t] * s[t + 1];
                        break;
                    case 5:  /* ���� */
                        t--;
                        s[t] = s[t] / s[t + 1];
                        break;
                    case 6:  /* ODD */
                        s[t] = s[t] % 2;
                        break;
                    case 8:  /* ���� */
                        t--;
                        s[t] = (s[t] == s[t + 1]);
                        break;
                    case 9:  /* ������ */
                        t--;
                        s[t] = (s[t] != s[t + 1]);
                        break;
                    case 10: /* С�� */
                        t--;
                        s[t] = (s[t] < s[t + 1]);
                        break;
                    case 11: /* ���ڵ��� */
                        t--;
                        s[t] = (s[t] >= s[t + 1]);
                        break;
                    case 12: /* ���� */
                        t--;
                        s[t] = (s[t] > s[t + 1]);
                        break;
                    case 13: /* С�ڵ��� */
                        t--;
                        s[t] = (s[t] <= s[t + 1]);
                        break;
                    case 14: /* ���ջ�� */
                        printf("%d ", s[t]);
                        t--;
                        break;
                    case 15: /* ������� */
                        printf("\n");
                        break;
                    case 16: /* ���� */
                        t++;
                        printf("? ");
                        scanf("%d", &s[t]);
                        break;
                }
                break;
                
            case LOD:
                t++;
                s[t] = s[base(i.l, b, s) + i.a];
                break;
                
            case STO:
                s[base(i.l, b, s) + i.a] = s[t];
                t--;
                break;
                
            case CAL:
                s[t + 1] = base(i.l, b, s);
                s[t + 2] = b;
                s[t + 3] = p;
                b = t + 1;
                p = i.a;
                break;
                
            case INT:
                t += i.a;
                break;
                
            case JMP:
                p = i.a;
                break;
                
            case JPC:
                if (s[t] == 0) {
                    p = i.a;
                }
                t--;
                break;
        }
    } while (p != 0);
    
    printf("\n=== END PL/0 ===\n");
}

/* ������ */
int main(int argc, char *argv[]) {
    char filename[256];
    
    printf("PL/0 Compiler (Flex & Bison version)\n");
    
    if (argc > 1) {
        strcpy(filename, argv[1]);
    } else {
        printf("Input PL/0 source file: ");
        scanf("%s", filename);
    }
    
    yyin = fopen(filename, "r");
    if (!yyin) {
        printf("Cannot open file %s\n", filename);
        return 1;
    }
    
    /* ��ʼ��ȫ�ֱ��� */
    cx = 0;
    level = 0;
    tx = 0;
    dx = 0;
    err_count = 0;
    
    printf("Compiling %s...\n", filename);
    yyparse();
    
    fclose(yyin);
    return 0;
}
