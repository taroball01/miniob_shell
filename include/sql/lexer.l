%{
/* definition part of c, this part will copy to lexer.yy.c */
/* we need token from yacc.tab.h*/
#include "sql/predicate/predicate.h"
#include "sql/yacc.tab.h"
#include "sql/parser.h"

extern int atoi();
extern double atof();
using namespace query_process_engine;
#define CONTEXT query_process_engine::get_context(yyscanner)
%}

/* ---- Options Part ---- */
/* noyywrap means if get EOF then won't change to a new input buffer */ 
%option noyywrap
/* http://westes.github.io/flex/manual/Bison-Bridge.html */
%option bison-bridge
/* https://www.cs.virginia.edu/~cr4bd/flex-manual/Reentrant-Detail.html#Reentrant-Detail */
%option reentrant

WHITE_SPACE           [\ \t\b\r\f]
DIGIT                 [0-9]

ID                    [A-Za-z_]+[A-Za-z0-9_]*
DOT                   \.
/* https://stackoverflow.com/questions/2039795/regular-expression-for-a-string-literal-in-flex-lex */
%%
\n                                ; 
\'([^\\\']|\\.)*\'                CONTEXT->str_stack_.emplace_back(yytext); return STRING;
{WHITE_SPACE}                     ; /* ignore whitespace*/
exit                              return EXIT;
select                            return SELECT;
from                              return FROM;
";"                               return SEMICOLON;
","                               return COMMA;
"*"                               return STAR;
"."                               return DOT;
where                             return WHERE;
"("                               return LBRACE;
")"                               return RBRACE;
not                               return NOT;
and                               return AND;
or                                return OR;
"="                               return EQ;
"<="                              return LE;
"!="                              return NE;
"<"                               return LT;
">="                              return GE;
">"                               return GT;
[\-]?{DIGIT}+                     yylval->number = atoi(yytext); return NUMBER;
[\-]?{DIGIT}+{DOT}{DIGIT}+        yylval->floats = (float)(atof(yytext)); return FLOAT;
{ID}                              CONTEXT->str_stack_.emplace_back(yytext); return ID;
.                                 printf("Unknown character [%c]\n",yytext[0]); return yytext[0];
%%

