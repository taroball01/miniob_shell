#pragma once
#include <memory>
#include <vector>
#include "relation/attribute.h"
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"

namespace query_process_engine {
class SelectQuery : public Query {
 private:
  std::vector<Attribute> attributes_;
  std::vector<std::string> relations_;
  std::unique_ptr<Predicate> conditions_;

 public:
  SelectQuery() = default;
  auto get_sql_type() const -> SqlType override { return SqlType::Select; }
  // we need to fill relations for some attributes
  auto get_attributes() -> std::vector<Attribute> & { return attributes_; }
  auto get_relations() const -> const std::vector<std::string> & { return relations_; }
  auto is_conditions_exist() const -> bool { return conditions_ != nullptr; }
  auto get_conditions() const -> Predicate & { return *conditions_; }
  auto add_attribute(const Attribute &attribute) -> void { attributes_.push_back(attribute); }
  auto add_relation(const std::string &relation) -> void { relations_.push_back(relation); }
  auto set_condition(std::unique_ptr<Predicate> &&pred) -> void { conditions_ = std::move(pred); }
  // This call will transfer ownership of conditions_
  auto transfer_conditions() -> std::unique_ptr<Predicate> { return std::move(conditions_); }
};
}  // namespace query_process_engine