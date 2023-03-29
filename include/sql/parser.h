#pragma once

#include <memory>
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

#include <string>
#include <vector>
#include "relation/attribute.h"
#include "relation/value/value.h"
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"

namespace query_process_engine {
struct ParserContext {
  std::unique_ptr<Query> query_;
  std::vector<std::string> str_stack_;
  std::vector<Attribute> attr_stack_;
  std::vector<std::unique_ptr<Value>> value_stack_;
  std::vector<std::unique_ptr<Operand>> operand_stack_;
  std::vector<std::unique_ptr<Predicate>> pred_stack_;
  auto clear() -> void {
    std::unique_ptr<Query>().swap(query_);
    str_stack_.clear();
    attr_stack_.clear();
    value_stack_.clear();
    operand_stack_.clear();
    pred_stack_.clear();
  }

  template <typename T>
  auto get_query() -> T * {
    return dynamic_cast<T *>(query_.get());
  }

  auto pop_str() -> std::string {
    std::string ret = str_stack_.back();
    str_stack_.pop_back();
    return ret;
  }

  auto pop_attr() -> Attribute {
    Attribute attr = attr_stack_.back();
    attr_stack_.pop_back();
    return attr;
  }

  auto pop_value() -> std::unique_ptr<Value> {
    std::unique_ptr<Value> pval = std::move(value_stack_.back());
    value_stack_.pop_back();
    return pval;
  }

  auto pop_predicate() -> std::unique_ptr<Predicate> {
    std::unique_ptr<Predicate> pred = std::move(pred_stack_.back());
    pred_stack_.pop_back();
    return pred;
  }

  auto pop_operand() -> std::unique_ptr<Operand> {
    std::unique_ptr<Operand> oprd = std::move(operand_stack_.back());
    operand_stack_.pop_back();
    return oprd;
  }
};

// switch buffer to str
void scan_string(const char *str, yyscan_t scanner);
/* interface to parse sql info into context
 *  @param sql: input sql statement
 *  @param context: parse result stores in context.query_, context also contains some supplymentary stacks
 *  @return: 0 if succeed
 */
int sql_parse(const std::string &sql, ParserContext &context);

ParserContext *get_context(yyscan_t scanner);
}  // namespace query_process_engine

void yyerror(yyscan_t scanner, const char *str);
