#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include "planner/plan_node.h"

namespace query_process_engine {
class Optimizer {
 public:
  using optimize_func_t = std::function<auto(std::unique_ptr<PlanNode>)->std::unique_ptr<PlanNode>>;

 private:
  std::vector<optimize_func_t> custom_optimzers_;

 private:
  enum class ChildDirection {
    Left,
    Right,
    Invalid,
  };

  struct FilterArrItem {
    // to be compatible with pair
    std::unique_ptr<FilterPlanNode> first;
    std::vector<std::string> second;
    FilterArrItem(std::unique_ptr<FilterPlanNode> ptr, std::vector<std::string> rel)
        : first(std::move(ptr)), second(std::move(rel)) {}
  };

 private:
  using join_trie_t = std::unordered_map<const BinaryJoinPlanNode *, std::unordered_map<std::string, ChildDirection>>;
  using filter_arr_t = std::vector<FilterArrItem>;
  auto build_join_trie(PlanNode &, join_trie_t &) -> const PlanNode *;
  auto push_down_filter(std::unique_ptr<PlanNode>, const join_trie_t &, filter_arr_t &) -> std::unique_ptr<PlanNode>;
  // return if relations all in one direction
  auto check_direction(const std::vector<std::string> &, const std::unordered_map<std::string, ChildDirection> &)
      -> ChildDirection;
  // build relations arr from predicate
  auto build_relations_from_predicate(Predicate &, std::vector<std::string> &) -> void;

 public:
  // currently only build a left-deep binary join tree
  auto choose_join_sequence(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode>;
  auto push_down_filter(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode>;

 public:
  auto optimize(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode>;
  auto register_optimzer(optimize_func_t) -> void;
};

}  // namespace query_process_engine