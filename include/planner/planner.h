#pragma once

#include "planner/plan_node.h"
#include "preprocess/stmt/select_stmt.h"
namespace query_process_engine {
class Planner {
 private:
  auto plan_select(SelectStmt &) -> std::unique_ptr<PlanNode>;

 public:
  auto plan_stmt(Statement &) -> std::unique_ptr<PlanNode>;
};
}  // namespace query_process_engine