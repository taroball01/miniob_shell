#include <iostream>
#include <vector>
#include "sql/parser.h"

extern int sql_parse(const char *, ParserContext&);
extern char lowercase(char);
extern const char* type2string(SqlType);
int main() {
  while (true) {
    std::cout<<">\t"<<std::flush;
    // read sql
    std::vector<char> sql;
    char nxt;
    bool semi = false;
    do {
      nxt = lowercase(std::cin.get());
      semi |= (nxt == ';');
      sql.emplace_back(nxt);
    } while (!(semi && nxt == '\n')); 
    sql.back() = '\0';
    ParserContext context;
    // pass to parser, store message in context
    int result = sql_parse(sql.data(), context);
    if (!result && context.query) {
      SqlType type = context.query->get_sql_type();
      std::cout<<type2string(type);
      delete context.query;
      context.query = nullptr;

      if (type == SqlType::EXIT) {
        std::cout<<"\nBye\n";
        return 0;
      }
    } 
    std::cout<<"\n";
  }
  return 0;
}