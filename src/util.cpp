#include <stdio.h>
#include "sql/parser.h"
#define CASE(LABEL) case LABEL: return #LABEL
const char* type2string(SqlType tp) {
  switch (tp) {
    CASE(SqlType::EXIT);
    CASE(SqlType::HELP);
    CASE(SqlType::DROP_TABLE);
    CASE(SqlType::SHOW_TABLES);
    CASE(SqlType::DESC_TABLE);
    CASE(SqlType::DROP_INDEX);
    CASE(SqlType::CREATE_INDEX);
    CASE(SqlType::INSERT);
    CASE(SqlType::CREATE_TABLE);
    CASE(SqlType::UPDATE);
    CASE(SqlType::DELETE);
    CASE(SqlType::DIRECT_SELECT);
  }
  return "unknown-type";
} 

char lowercase(char ch) {
  static const char distance = 'a' - 'A';
  return (ch >= 'A' && ch <= 'Z' ? ch + distance : ch);
}