#include "relation/schema.h"
#include <algorithm>
#include <cassert>
namespace query_process_engine {

Schema::Schema(const std::vector<SchemaItem> &arr) : schema_(arr) {
  int id = 0;
  for (auto &item : schema_) {
    rel_map_[item.relation_][item.attribute_] = id;
    attr_map_[item.attribute_][item.relation_] = id;
    ++id;
  }
}

auto Schema::append_schema(const SchemaItem &it) -> void {
  int pos = schema_.size();
  schema_.emplace_back(it);
  rel_map_[it.relation_][it.attribute_] = pos;
  attr_map_[it.attribute_][it.relation_] = pos;
}

auto Schema::is_attribute_exists(const std::string &relation, const std::string &attribute) const -> bool {
  if (rel_map_.find(relation) == rel_map_.end()) {
    return false;
  } else {
    auto &mp = rel_map_.at(relation);
    return mp.find(attribute) != mp.end();
  }
}

auto Schema::get_attribute_schema(const std::string &relation, const std::string &attribute) const
    -> const SchemaItem & {
  int id = -1;
  if (rel_map_.find(relation) == rel_map_.end()) {
    id = -1;
  } else {
    auto &mp = rel_map_.at(relation);
    id = mp.find(attribute) == mp.end() ? -1 : mp.at(attribute);
  }
  assert(id != -1);
  return schema_.at(id);
}

auto Schema::get_relation_schema(const std::string &relation) const -> std::vector<SchemaItem> {
  std::vector<SchemaItem> ret;
  if (rel_map_.find(relation) != rel_map_.end()) {
    auto &mp = rel_map_.at(relation);
    std::vector<std::pair<std::string, int>> rel_map_arr(mp.begin(), mp.end());
    std::sort(rel_map_arr.begin(), rel_map_arr.end(),
              [](const std::pair<std::string, int> &lhs, const std::pair<std::string, int> &rhs) -> bool {
                return lhs.second < rhs.second;
              });
    for (auto &[_, v] : rel_map_arr) {
      ret.emplace_back(schema_.at(v));
    }
  }
  return ret;
}

auto Schema::count_relation(const std::string &attribute) const -> int {
  return attr_map_.find(attribute) == attr_map_.end() ? 0 : attr_map_.at(attribute).size();
}

auto Schema::get_relation(const std::string &attribute) const -> std::string {
  if (1 != count_relation(attribute)) {
    return {};
  } else {
    return attr_map_.at(attribute).begin()->first;
  }
}

auto Schema::get_relations() const -> std::vector<std::string> {
  std::vector<std::string> ret;
  for (auto &[k, _] : rel_map_) {
    ret.emplace_back(k);
  }
  return ret;
}
// get attribute id
auto Schema::get_attribute_id(const std::string &relation, const std::string &attribute) const -> int {
  return rel_map_.at(relation).at(attribute);
}
// clear current
auto Schema::clear() -> void {
  schema_.clear();
  rel_map_.clear();
  attr_map_.clear();
}

}  // namespace query_process_engine