#pragma once

#include <vector>
#include "relation/value/value.h"
#include "sql/query/query.h"
namespace query_process_engine {
class InsertQuery : public Query {
private:
  std::string relation_;
  std::vector<std::unique_ptr<Value>> value_arr_;
public:
  explicit InsertQuery(const std::string& rel) : relation_(rel) {}
  auto get_sql_type() const -> SqlType {
    return SqlType::Insert;
  }
  auto append_value(std::unique_ptr<Value> pv) -> void {
    value_arr_.emplace_back(std::move(pv));
  }
  auto get_relation_name() const -> const std::string& {
    return relation_;
  }
  auto get_value_arr() -> std::vector<std::unique_ptr<Value>>& {
    return value_arr_;
  }
};

} // namespace query_process