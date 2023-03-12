#include <iostream>
#include <vector>
#include "sql/parser.h"

extern char lowercase(char);
#define CASE(LABEL) case LABEL: return #LABEL
const char* type2string(SqlType tp) {
  switch (tp) {
    CASE(SqlType::Exit);
    CASE(SqlType::Select);
    default: return "unknow-type";
  }
} 

char lowercase(char ch) {
  static const char distance = 'a' - 'A';
  return (ch >= 'A' && ch <= 'Z' ? ch + distance : ch);
}

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
    if (!result && context.query_) {
      SqlType type = context.query_->get_sql_type();
      std::cout<<type2string(type);

      if (type == SqlType::Exit) {
        std::cout<<"\nBye\n";
        return 0;
      }
    } 
    std::cout<<"\n";
  }
  return 0;
}