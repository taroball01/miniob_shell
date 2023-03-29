#pragma once

#include "common/storage_manager.h"
#include "executor/operator.h"
#include "planner/plan_node.h"
namespace query_process_engine {
class Executor {
 private:
  ITranscationalStorageManager &ts_manager_;

 public:
  explicit Executor(ITranscationalStorageManager &tsm) : ts_manager_(tsm) {}
  auto build_physical_operator(PlanNode &) -> std::unique_ptr<PhysicalOperator>;
  auto execute(PhysicalOperator &) -> std::vector<Tuple>;
};
}  // namespace query_process_engine