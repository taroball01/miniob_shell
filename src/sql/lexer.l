%{
/* definition part of c */
#include <memory>

#include "sql/parser.h"
#include "sql/yacc.tab.h"

extern int atoi();
extern double atof();

%}

/* ---- Options Part ---- */
/* noyywrap means if get EOF then won't change to a new input buffer */ 
%option noyywrap
/* http://westes.github.io/flex/manual/Bison-Bridge.html */
%option bison-bridge
/* https://www.cs.virginia.edu/~cr4bd/flex-manual/Reentrant-Detail.html#Reentrant-Detail */
%option reentrant

WHITE_SPACE      [\ \t\b\r\f]
DIGIT            [0-9]

ID               [A-Za-z_]+[A-Za-z0-9_]*
DOT              \.
SINGLE_QUOTE     \'
DOUBLE_QUOTE     \"

%%
{WHITE_SPACE}     ; /* ignore whitespace*/
\n                ; 

exit                                                 return EXIT;
";"                                                  return SEMICOLON;
help                                                 return HELP;
show                                                 return SHOW;
tables                                               return TABLES;
drop                                                 return DROP;
table                                                return TABLE;
desc                                                 return DESC;
index                                                return INDEX;
create                                               return CREATE;
on                                                   return ON;
"("                                                  return LBRACE;
")"                                                  return RBRACE;
","                                                  return COMMA;
int                                                  return INT_T;
char                                                 return STRING_T;
float                                                return FLOAT_T;
date                                                 return DATE_T;
insert                                               return INSERT;
into                                                 return INTO;
values                                               return VALUES;
[\-]?{DIGIT}+                                        yylval->number = atoi(yytext); return NUMBER;
[\-]?{DIGIT}+{DOT}{DIGIT}+                           yylval->floats = (float)(atof(yytext)); return FLOAT;
{SINGLE_QUOTE}.*{SINGLE_QUOTE} |
{DOUBLE_QUOTE}.*{DOUBLE_QUOTE}                       yylval->string = strdup(yytext); return STRING;
update                                               return UPDATE;
set                                                  return SET;
and                                                  return AND;
or                                                   return OR;
not                                                  return NOT;
"="                                                  return EQ;
"<="                                                 return LE;
"<>"                                                 return NE;
"<"                                                  return LT;
">="                                                 return GE;
">"                                                  return GT;
{DOT}                                                return DOT;
delete                                               return DELETE;
from                                                 return FROM;
select                                               return SELECT;
"*"                                                  return STAR;
where                                                return WHERE;

{ID}                                                 yylval->string=strdup(yytext); return ID; /* ID should put on last or it will override other rules*/
.                                                    printf("Unknown character [%c]\n",yytext[0]); return yytext[0];
%%

void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}