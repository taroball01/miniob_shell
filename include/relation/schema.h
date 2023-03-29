#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "relation/value/value.h"

namespace query_process_engine {

struct SchemaItem {
  std::string relation_;
  std::string attribute_;
  ValueType type_;
};
class Schema {
 private:
  std::vector<SchemaItem> schema_;
  // [relation, [attribute, id]]
  std::unordered_map<std::string, std::unordered_map<std::string, int>> rel_map_;
  // [attribute, [relation, id]]
  std::unordered_map<std::string, std::unordered_map<std::string, int>> attr_map_;

 public:
  explicit Schema(const std::vector<SchemaItem> &);
  Schema() = default;
  // append a schema item
  auto append_schema(const SchemaItem &ap) -> void;
  // check schema exists
  auto is_attribute_exists(const std::string &relation, const std::string &attribute) const -> bool;
  // get schema item by relation, attribute
  auto get_attribute_schema(const std::string &relation, const std::string &attribute) const -> const SchemaItem &;
  // get table schema
  auto get_relation_schema(const std::string &relation) const -> std::vector<SchemaItem>;
  // count how many relations own this attribute
  auto count_relation(const std::string &attribute) const -> int;
  // return unique relation from an attribute name
  // if there are multiple relations, return empty string;
  auto get_relation(const std::string &attribute) const -> std::string;
  // return inner schema_ array
  inline auto get_schema_array() const -> const std::vector<SchemaItem> & { return schema_; }
  // get all relations related
  auto get_relations() const -> std::vector<std::string>;
  // get attribute id
  auto get_attribute_id(const std::string &relation, const std::string &attribute) const -> int;
  // clear current
  auto clear() -> void;
};
}  // namespace query_process_engine