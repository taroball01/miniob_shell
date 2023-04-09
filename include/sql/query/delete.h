#pragma once

#include <memory>
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"

namespace query_process_engine {
class DeleteQuery : public Query {
private:
  std::string relation_;
  std::unique_ptr<Predicate> conditions_;
public:
  explicit DeleteQuery(const std::string& rel) : relation_(rel) {}
  auto get_sql_type() const -> SqlType { return SqlType::Delete; }

  auto set_condition(std::unique_ptr<Predicate> &&pred) -> void { conditions_ = std::move(pred); }
  auto get_conditions() const -> Predicate & { return *conditions_; }
  auto transfer_conditions() -> std::unique_ptr<Predicate> { return std::move(conditions_); }
  auto get_relation() const -> const std::string& { return relation_; }
};
} // namespace query_process_engine