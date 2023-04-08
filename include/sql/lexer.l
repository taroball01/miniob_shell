%{
/* definition part of c, this part will copy to lexer.yy.c */
/* we need token from yacc.tab.h*/
#include "sql/predicate/predicate.h"
#include "sql/yacc.tab.h"
#include "sql/parser.h"
#include "common/parser_error/unknown_character.h"
#include "common/parser_error/unexpected_token.h"

extern int atoi();
extern double atof();
using namespace query_process_engine;
#define CONTEXT query_process_engine::get_context(yyscanner)

#define YY_USER_ACTION                                             \
    yylloc->first_line = yylloc->last_line;                          \
    yylloc->first_column = yylloc->last_column;                      \
    if (yylloc->last_line == yylineno)                               \
      yylloc->last_column += yyleng;                                 \
    else {                                                           \
      yylloc->last_line = yylineno;                                  \
      yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
    }
%}

/* ---- Options Part ---- */
/* noyywrap means if get EOF then won't change to a new input buffer */ 
%option noyywrap
/* http://westes.github.io/flex/manual/Bison-Bridge.html */
%option bison-bridge
/* https://www.cs.virginia.edu/~cr4bd/flex-manual/Reentrant-Detail.html#Reentrant-Detail */
%option reentrant
/* add location part */
%option bison-locations 
%option yylineno

WHITE_SPACE           [\ \t\b\r\f]
DIGIT                 [0-9]

ID                    [A-Za-z_]+[A-Za-z0-9_]*
DOT                   \.
/* https://stackoverflow.com/questions/2039795/regular-expression-for-a-string-literal-in-flex-lex */
%%
\n                                ; 
\'([^\\\']|\\.)*\'                CONTEXT->str_stack_.emplace_back(yytext); return STRING;
{WHITE_SPACE}                     ; /* ignore whitespace*/
[Ee][Xx][Ii][Tt]                  return EXIT;
[Ss][Ee][Ll][Ee][Cc][Tt]          return SELECT;
[Ff][Rr][Oo][Mm]                  return FROM;
";"                               return SEMICOLON;
","                               return COMMA;
"*"                               return STAR;
"."                               return DOT;
[Ww][Hh][Ee][Rr][Ee]              return WHERE;

[Ss][Hh][Oo][Ww]                  return SHOW;
[Tt][Aa][Bb][Ll][Ee][Ss]          return TABLES;
[Dd][Ee][Ss][Cc]                  return DESC;

[Cc][Rr][Ee][Aa][Tt][Ee]          return CREATE;
[Tt][Aa][Bb][Ll][Ee]              return TABLE;

int                               return INT_T;
string                            return STRING_T;
date                              return DATE_T;
float                             return FLOAT_T;

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
.                                 throw UnknownCharacter(yytext[0], yylloc->first_line, yylloc->first_column);
%%

void yyerror(YYLTYPE* yyllocp, yyscan_t yyscanner, const char* msg) { 
  int row = yyllocp->first_line;
  int col = yyllocp->first_column;
  throw UnexpectedToken(msg, row, col);
}
