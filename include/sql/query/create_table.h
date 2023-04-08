#pragma once

#include <vector>
#include "relation/schema.h"
#include "sql/query/query.h"
namespace query_process_engine {
class CreateTable : public Query {
private:
  std::string rel_name_;
  std::vector<SchemaItem> schema_;
public:
  explicit CreateTable(const std::string& rel) : rel_name_(rel) {}
  auto get_sql_type() const -> SqlType override {
    return SqlType::CreateTable;
  }
  auto append_schema_item(SchemaItem it) -> void {
    it.relation_ = rel_name_;
    schema_.emplace_back(it);
  }
  auto get_relation_name() const -> const std::string& {
    return rel_name_;
  }
  auto get_schema_array() const -> const std::vector<SchemaItem>& {
    return schema_;
  }
};
} // namespace query_process_engine
