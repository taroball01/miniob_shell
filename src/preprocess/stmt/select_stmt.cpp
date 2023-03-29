#include "preprocess/stmt/select_stmt.h"
#include "sql/predicate/predicate.h"

namespace query_process_engine {
SelectStmt::SelectStmt(const Schema &schema, const std::vector<Attribute> &attributes,
                       std::unique_ptr<Predicate> condition)
    : schema_(schema), attributes_(attributes) {
  split_and(std::move(condition));
}

auto SelectStmt::split_and(std::unique_ptr<Predicate> ptr) -> void {
  if (nullptr == ptr) {
    return ;
  }
  
  if (ptr->get_node_type() == PredicateType::CNT_AND) {
    auto &pred = dynamic_cast<PredicateAnd &>(*ptr);
    split_and(pred.transfer_lchild());
    split_and(pred.transfer_rchild());
  } else {
    conditions_.emplace_back(std::move(ptr));
  }
}

auto SelectStmt::get_schema() -> const Schema & { return schema_; }

auto SelectStmt::get_attributes() -> const std::vector<Attribute> & { return attributes_; }

auto SelectStmt::get_conditions() -> std::vector<std::unique_ptr<Predicate>> & { return conditions_; }

auto SelectStmt::set_conditions(std::unique_ptr<Predicate> ptr) -> void {
  conditions_.clear();
  split_and(std::move(ptr));
}
}  // namespace query_process_engine