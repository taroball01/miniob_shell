#include "executor/executor.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include "executor/operator.h"
#include "planner/plan_node.h"
namespace query_process_engine {

auto Executor::build_physical_operator(PlanNode &plan) -> std::unique_ptr<PhysicalOperator> {
  switch (plan.get_plan_node_type()) {
    case PlanNodeType::BinaryJoin: {
      auto &cur = dynamic_cast<BinaryJoinPlanNode &>(plan);
      auto lchild = build_physical_operator(cur.get_lchild());
      auto rchild = build_physical_operator(cur.get_rchild());
      return std::make_unique<NestLoopJoinOperator>(cur, std::move(lchild), std::move(rchild));
    }
    case PlanNodeType::Filter: {
      auto &cur = dynamic_cast<FilterPlanNode &>(plan);
      auto child = build_physical_operator(cur.get_child());
      return std::make_unique<FilterOperator>(cur, std::move(child));
    }
    case PlanNodeType::Projection: {
      auto &cur = dynamic_cast<ProjectionPlanNode &>(plan);
      auto child = build_physical_operator(cur.get_child());
      return std::make_unique<ProjectionOperator>(cur, std::move(child));
    }
    case PlanNodeType::TableScan: {
      auto &cur = dynamic_cast<TableScanPlanNode &>(plan);
      return std::make_unique<TableScanOperator>(ts_manager_, cur);
    }
    default: {
      throw std::logic_error("Executor::build_physical_operator");
    }
  }
}

auto Executor::execute(PhysicalOperator &op) -> std::vector<Tuple> {
  std::vector<Tuple> result;
  if (op.open()) {
    Tuple tp;
    while (op.get_next(tp)) {
      result.emplace_back(tp);
      tp.clear();
    }
  }
  return result;
}

}  // namespace query_process_engine