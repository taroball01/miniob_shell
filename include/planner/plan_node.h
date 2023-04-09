#pragma once
#include <memory>
#include <vector>
#include "preprocess/stmt/insert_stmt.h"
#include "relation/attribute.h"
#include "relation/schema.h"
#include "sql/predicate/predicate.h"
#include "sql/query/insert.h"
namespace query_process_engine {
enum class PlanNodeType {
  TableScan,
  MultiJoin,
  BinaryJoin,
  Filter,
  Projection,

  Insert,
};

class PlanNode {
 protected:
  Schema schema_;

 public:
  explicit PlanNode(Schema sch) : schema_(sch) {}
  PlanNode() = default;
  virtual ~PlanNode() = default;
  virtual auto get_plan_node_type() const -> PlanNodeType = 0;
  auto get_output_schema() const -> const Schema & { return schema_; }
};

class TableScanPlanNode : public PlanNode {
 private:
  std::string relation_;

 public:
  TableScanPlanNode(const std::string &rel, const std::vector<SchemaItem> &sch)
      : PlanNode(Schema{sch}), relation_(rel) {}
  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::TableScan; }
  auto get_relation() const -> const std::string & { return relation_; }
};

class MultiJoinPlanNode : public PlanNode {
 private:
  std::vector<std::unique_ptr<PlanNode>> children_;

 public:
  // default initialize output schema
  MultiJoinPlanNode() = default;
  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::MultiJoin; }
  auto append_child(std::unique_ptr<PlanNode> ptr) -> void {
    for (auto &item : ptr->get_output_schema().get_schema_array()) {
      schema_.append_schema(item);
    }
    children_.emplace_back(std::move(ptr));
  }
  auto get_children() -> std::vector<std::unique_ptr<PlanNode>> & { return children_; }
};

class BinaryJoinPlanNode : public PlanNode {
 private:
  std::unique_ptr<PlanNode> lchild_;
  std::unique_ptr<PlanNode> rchild_;

 private:
  auto build_output_schema() -> void {
    schema_.clear();
    for (auto &item : lchild_->get_output_schema().get_schema_array()) {
      schema_.append_schema(item);
    }
    for (auto &item : rchild_->get_output_schema().get_schema_array()) {
      schema_.append_schema(item);
    }
  }

 public:
  BinaryJoinPlanNode(std::unique_ptr<PlanNode> lc, std::unique_ptr<PlanNode> rc)
      : PlanNode(), lchild_(std::move(lc)), rchild_(std::move(rc)) {
    build_output_schema();
  }
  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::BinaryJoin; }
  auto get_lchild() -> PlanNode & { return *lchild_; }
  auto get_rchild() -> PlanNode & { return *rchild_; }
  auto transfer_lchild() -> std::unique_ptr<PlanNode> { return std::move(lchild_); }
  auto transfer_rchild() -> std::unique_ptr<PlanNode> { return std::move(rchild_); }
  auto set_lchild(std::unique_ptr<PlanNode> ptr) -> void {
    lchild_ = std::move(ptr);
    build_output_schema();
  }
  auto set_rchild(std::unique_ptr<PlanNode> ptr) -> void {
    rchild_ = std::move(ptr);
    build_output_schema();
  }
};

class FilterPlanNode : public PlanNode {
 private:
  std::unique_ptr<PlanNode> child_;
  std::unique_ptr<Predicate> condition_;

 private:
  auto build_output_schema() -> void { schema_ = child_->get_output_schema(); }

 public:
  FilterPlanNode(std::unique_ptr<PlanNode> child, std::unique_ptr<Predicate> condition)
      : PlanNode(Schema{child->get_output_schema()}), child_(std::move(child)), condition_(std::move(condition)) {}

  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::Filter; }

  auto get_child() -> PlanNode & { return *child_; }
  auto get_condition() const -> Predicate & { return *condition_; }
  auto transfer_child() -> std::unique_ptr<PlanNode> { return std::move(child_); }
  auto set_child(std::unique_ptr<PlanNode> ptr) -> void {
    child_ = std::move(ptr);
    build_output_schema();
  }
};

class ProjectionPlanNode : public PlanNode {
 private:
  std::unique_ptr<PlanNode> child_;
  std::vector<Attribute> attributes_;

 private:
  auto build_output_schema() -> void {
    schema_.clear();
    auto &shc = child_->get_output_schema();
    for (auto &attr : attributes_) {
      schema_.append_schema(shc.get_attribute_schema(attr.get_relation(), attr.get_attribute()));
    }
  }

 public:
  ProjectionPlanNode(std::unique_ptr<PlanNode> child, const std::vector<Attribute> &attributes)
      : PlanNode(), child_(std::move(child)), attributes_(attributes) {
    build_output_schema();
  }

  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::Projection; }

  auto get_child() -> PlanNode & { return *child_; }
  auto transfer_child() -> std::unique_ptr<PlanNode> { return std::move(child_); }
  auto set_child(std::unique_ptr<PlanNode> ptr) -> void {
    child_ = std::move(ptr);
    build_output_schema();
  }
};

class InsertPlanNode : public PlanNode {
 private:
  std::vector<std::unique_ptr<Value>> value_arr_;
  std::string relation_;

 public:
  explicit InsertPlanNode(InsertStmt &stmt)
      : PlanNode(Schema(std::vector<SchemaItem>{SchemaItem{"", "affected_rows", ValueType::VT_INT}})),
        value_arr_(std::move(stmt.get_query().get_value_arr())),
        relation_(stmt.get_query().get_relation_name()) {}

  auto get_plan_node_type() const -> PlanNodeType override { return PlanNodeType::Insert; }
  auto get_relation_name() const -> const std::string & { return relation_; }
  auto get_value_arr() -> std::vector<std::unique_ptr<Value>> & { return value_arr_; }
};
}  // namespace query_process_engine