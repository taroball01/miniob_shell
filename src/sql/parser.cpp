#include "sql/parser.h"
// clang-format off
#include "sql/yacc.tab.h"
#include "sql/lexer.yy.h"
// clang-format on
namespace query_process_engine {
void scan_string(const char *str, yyscan_t scanner) { yy_switch_to_buffer(yy_scan_string(str, scanner), scanner); }

// return 0 if parse succeeds.
int sql_parse(const std::string &sql, ParserContext &context) {
  context.clear();
  yyscan_t scanner;
  yylex_init_extra(&context, &scanner);
  scan_string(sql.data(), scanner);
  int result = yyparse(scanner);
  yylex_destroy(scanner);
  return result;
}

ParserContext *get_context(yyscan_t scanner) { return (ParserContext *)yyget_extra(scanner); }
}  // namespace query_process_engine
