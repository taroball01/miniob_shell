#include "sql/parser.h"
// clang-format off
#include "common/parser_error/parser_error_info.h"
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
  // seems it's a bug when using yy_scan_string, it won't init lineno and column
  // https://stackoverflow.com/questions/14747410/flex-2-5-35-yy-scan-buffer-not-initializing-line-and-column-numbers
  yyset_lineno(1, scanner);
  yyset_column(1, scanner);

  int result = -1;
  try {
    result = yyparse(scanner);
  } catch (...) {
    yylex_destroy(scanner);
    // pass to next
    throw;
  }
  yylex_destroy(scanner);
  return result;
}

ParserContext *get_context(yyscan_t scanner) { return (ParserContext *)yyget_extra(scanner); }
}  // namespace query_process_engine
