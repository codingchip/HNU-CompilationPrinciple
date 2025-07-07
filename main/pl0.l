/* pl0.l - PL/0 词法分析器 */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pl0.h"
#include "pl0.tab.h"

int line_no = 1;
int col_no = 1;

void count() {
    int i;
    for (i = 0; yytext[i] != '\0'; i++) {
        if (yytext[i] == '\n') {
            col_no = 1;
            line_no++;
        } else if (yytext[i] == '\t') {
            col_no += 8 - (col_no % 8);
        } else {
            col_no++;
        }
    }
}

%}

%option noyywrap
%option case-insensitive

DIGIT    [0-9]
LETTER   [a-zA-Z]
ID       {LETTER}({LETTER}|{DIGIT})*
NUMBER   {DIGIT}+
COMMENT  \{[^}]*\}

%%

{COMMENT}       { count(); /* 忽略注释 */ }
[ \t\r]         { count(); /* 忽略空白字符 */ }
\n              { count(); }

"CONST"         { count(); return CONST; }
"VAR"           { count(); return VAR; }
"PROCEDURE"     { count(); return PROCEDURE; }
"CALL"          { count(); return CALL; }
"BEGIN"         { count(); return BEGIN_SYM; }
"END"           { count(); return END; }
"IF"            { count(); return IF; }
"THEN"          { count(); return THEN; }
"WHILE"         { count(); return WHILE; }
"DO"            { count(); return DO; }
"ODD"           { count(); return ODD; }
"READ"          { count(); return READ; }
"WRITE"         { count(); return WRITE; }

":="            { count(); return ASSIGN; }
"="             { count(); return EQ; }
"#"             { count(); return NE; }
"<"             { count(); return LT; }
"<="            { count(); return LE; }
">"             { count(); return GT; }
">="            { count(); return GE; }
"+"             { count(); return PLUS; }
"-"             { count(); return MINUS; }
"*"             { count(); return TIMES; }
"/"             { count(); return SLASH; }
"("             { count(); return LPAREN; }
")"             { count(); return RPAREN; }
","             { count(); return COMMA; }
";"             { count(); return SEMICOLON; }
"."             { count(); return PERIOD; }

{ID}            { 
                    count(); 
                    yylval.ident = (char*)malloc(strlen(yytext) + 1);
                    strcpy(yylval.ident, yytext);
                    return IDENT; 
                }

{NUMBER}        { 
                    count(); 
                    yylval.number = atoi(yytext);
                    return NUMBER; 
                }

.               { 
                    count();
                    printf("Error: Unexpected character '%s' at line %d, column %d\n", 
                           yytext, line_no, col_no);
                    return yytext[0];
                }

%%