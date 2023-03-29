#include "sql/parser.h"

void yyerror(yyscan_t scanner, const char *str) { printf("parse sql failed. error=%s\n", str); }