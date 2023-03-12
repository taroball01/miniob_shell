#include "sql/parser.h"
#include "sql/yacc.tab.h"
#include "sql/lexer.yy.h"

void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}

// return 0 if parse succeeds.
int sql_parse(const std::string& sql, ParserContext& context) {
  context.clear();
  yyscan_t scanner;
  yylex_init_extra(&context, &scanner);
  scan_string(sql.data(), scanner);
  int result = yyparse(scanner);
  yylex_destroy(scanner);
  return result;
}

void yyerror(yyscan_t scanner, const char *str) {
  printf("parse sql failed. error=%s\n", str);
}

ParserContext *get_context(yyscan_t scanner) {
  return (ParserContext *)yyget_extra(scanner);
}