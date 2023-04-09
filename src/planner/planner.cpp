
#include "planner/planner.h"
#include <memory>
#include "planner/plan_node.h"
#include "preprocess/stmt/delete_stmt.h"
#include "sql/query/query.h"
namespace query_process_engine {
auto Planner::plan_stmt(Statement &st) -> std::unique_ptr<PlanNode> {
  switch (st.get_sql_type()) {
    case SqlType::Select: {
      return plan_select(dynamic_cast<SelectStmt &>(st));
    }
    case SqlType::Insert: {
      return plan_insert(dynamic_cast<InsertStmt &>(st));
    }
    case SqlType::Delete: {
      return plan_delete(dynamic_cast<DeleteStmt &>(st));
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

auto Planner::plan_insert(InsertStmt &stmt) -> std::unique_ptr<PlanNode> {
  return std::make_unique<InsertPlanNode>(stmt);
}

auto Planner::plan_delete(DeleteStmt& stmt) -> std::unique_ptr<PlanNode> {
  auto& query = stmt.get_query();
  auto ts = std::make_unique<TableScanPlanNode>(query.get_relation(), stmt.get_relation_schema());
  auto filter = std::make_unique<FilterPlanNode>(std::move(ts), query.transfer_conditions());
  return std::make_unique<DeletePlanNode>(std::move(filter));
}
}  // namespace query_process_engine