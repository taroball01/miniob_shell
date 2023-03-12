#pragma once
#include "sql/query/query.h"
#include <vector>
#include <memory>
#include "sql/query/attribute.h"
#include "sql/query/predicate.h"

class SelectQuery : public Query{
private:
  std::vector<Attribute> attributes_;
  std::vector<std::string> relations_;
  std::unique_ptr<Predicate> conditions_;
public:
  SelectQuery() = default;
  auto get_sql_type() const -> SqlType override {
    return SqlType::Select;
  }
  auto get_attributes() const -> const std::vector<Attribute>& {
    return attributes_;
  }
  auto get_relations() const -> const std::vector<std::string>& {
    return relations_;
  }
  auto get_conditions() const -> const std::unique_ptr<Predicate>& {
    return conditions_;
  }
  auto add_attribute(const Attribute& attribute) -> void {
    attributes_.push_back(attribute);
  }
  auto add_relation(const std::string& relation) -> void {
    relations_.push_back(relation);
  }
  auto set_condition(std::unique_ptr<Predicate>&& pred) -> void {
    conditions_ = std::move(pred);
  }
};