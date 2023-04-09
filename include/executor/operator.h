#pragma once

#include <memory>
#include "common/storage_manager.h"
#include "planner/plan_node.h"
#include "relation/tuple.h"

namespace query_process_engine {

class PhysicalOperator {
 public:
  PhysicalOperator() = default;
  virtual ~PhysicalOperator() = default;
  virtual auto open() -> bool = 0;
  // return true if get data, false if no more data
  virtual auto get_next(Tuple &) -> bool = 0;
  virtual auto close() -> void = 0;
  virtual auto get_plan() const -> const PlanNode & = 0;
};

class TableScanOperator : public PhysicalOperator {
 private:
  ITranscationalStorageManager &ts_manager_;
  TableScanPlanNode &plan_;
  std::shared_ptr<TupleId> p_next_id_;

 public:
  TableScanOperator(ITranscationalStorageManager &tsm, TableScanPlanNode &pl)
      : ts_manager_(tsm), plan_(pl), p_next_id_(nullptr) {}
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

class FilterOperator : public PhysicalOperator {
 private:
  FilterPlanNode &plan_;
  std::unique_ptr<PhysicalOperator> child_;

 public:
  FilterOperator(FilterPlanNode &plan, std::unique_ptr<PhysicalOperator> child)
      : plan_(plan), child_(std::move(child)) {}
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

class ProjectionOperator : public PhysicalOperator {
 private:
  ProjectionPlanNode &plan_;
  std::unique_ptr<PhysicalOperator> child_;

 public:
  ProjectionOperator(ProjectionPlanNode &plan, std::unique_ptr<PhysicalOperator> child)
      : plan_(plan), child_(std::move(child)) {}
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

class NestLoopJoinOperator : public PhysicalOperator {
 private:
  BinaryJoinPlanNode &plan_;
  std::unique_ptr<PhysicalOperator> lchild_;
  std::unique_ptr<PhysicalOperator> rchild_;
  Tuple l_tp_;

 public:
  NestLoopJoinOperator(BinaryJoinPlanNode &plan, std::unique_ptr<PhysicalOperator> lchild,
                       std::unique_ptr<PhysicalOperator> rchild);
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

class InsertOperator : public PhysicalOperator {
 private:
  ITranscationalStorageManager &ts_manager_;
  InsertPlanNode &plan_;
  int cnt_{0};

 public:
  InsertOperator(ITranscationalStorageManager &tsm, InsertPlanNode &pl) : ts_manager_(tsm), plan_(pl) {}
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

class DeleteOperator : public PhysicalOperator {
 private:
  ITranscationalStorageManager &ts_manager_;
  DeletePlanNode &plan_;
  std::unique_ptr<PhysicalOperator> child_;
  int cnt_{0};

 public:
  DeleteOperator(ITranscationalStorageManager &tsm, DeletePlanNode &pl, std::unique_ptr<PhysicalOperator> child) 
  : ts_manager_(tsm), plan_(pl), child_(std::move(child)) {}
  auto open() -> bool override;
  auto get_next(Tuple &) -> bool override;
  auto close() -> void override;
  auto get_plan() const -> const PlanNode & override { return plan_; }
};

}  // namespace query_process_engine