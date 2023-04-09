#pragma once

#include "preprocess/stmt/statement.h"
#include "relation/attribute.h"
#include "relation/schema.h"
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"

namespace query_process_engine {
class SelectStmt : public Statement {
 private:
  Schema schema_;
  std::vector<Attribute> attributes_;
  std::vector<std::unique_ptr<Predicate>> conditions_;

 private:
  // split predicate into a array by `and`
  auto split_and(std::unique_ptr<Predicate>) -> void;

 public:
  SelectStmt(const Schema &, const std::vector<Attribute> &, std::unique_ptr<Predicate>);

  auto get_sql_type() const -> SqlType override { return SqlType::Select; }
  auto get_schema() -> const Schema &;
  auto get_attributes() -> const std::vector<Attribute> &;
  auto get_conditions() -> std::vector<std::unique_ptr<Predicate>> &;
  auto set_conditions(std::unique_ptr<Predicate>) -> void;
};
}  // namespace query_process_engine