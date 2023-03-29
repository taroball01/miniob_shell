
#include "planner/planner.h"
#include <memory>
#include "planner/plan_node.h"
#include "sql/query/query.h"
namespace query_process_engine {
auto Planner::plan_stmt(Statement &st) -> std::unique_ptr<PlanNode> {
  switch (st.get_sql_type()) {
    case SqlType::Select: {
      return plan_select(dynamic_cast<SelectStmt &>(st));
    }
    default:
      return nullptr;
  }
}

auto Planner::plan_select(SelectStmt &stmt) -> std::unique_ptr<PlanNode> {
  auto mtj = std::make_unique<MultiJoinPlanNode>();
  for (auto &rel : stmt.get_schema().get_relations()) {
    mtj->append_child(std::make_unique<TableScanPlanNode>(rel, stmt.get_schema().get_relation_schema(rel)));
  }
  std::unique_ptr<PlanNode> ptr = std::move(mtj);
  for (auto &pred : stmt.get_conditions()) {
    ptr = std::make_unique<FilterPlanNode>(std::move(ptr), std::move(pred));
  }
  ptr = std::make_unique<ProjectionPlanNode>(std::move(ptr), stmt.get_attributes());
  return ptr;
}

}  // namespace query_process_engine