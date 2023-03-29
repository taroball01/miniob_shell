#pragma once

#include <memory>
#include "common/result_printer.h"
#include "common/storage_manager.h"
#include "preprocess/stmt/select_stmt.h"
#include "preprocess/stmt/statement.h"
#include "relation/schema.h"
#include "relation/value/value.h"
#include "sql/predicate/predicate.h"
#include "sql/query/select.h"
namespace query_process_engine {

class Preprocessor {
 private:
  ITranscationalStorageManager &ts_manager_;
  ResultPrinter &printer_;

 private:
  auto resolve_attribute_item(Attribute &, const Schema &) -> bool;
  auto resolve_operand_type(Operand &, const Schema &) -> ValueType;
  auto is_valid_compare(Operand &, Operand &, ValueType, ValueType) -> bool;

 public:
  // resolve relations, output schema into second parameter
  auto resolve_relations(const std::vector<std::string> &, Schema &) -> bool;
  // resolve attributes according to schema
  auto resolve_attributes(std::vector<Attribute> &, const Schema &) -> bool;
  // resolve attributes according to schema, check type
  auto resolve_predicate_leaves(Predicate &, const Schema &) -> bool;
  // precompute simple leaves into value to simplify the tree
  auto pre_compute_simple_leaves(std::unique_ptr<Predicate>) -> std::unique_ptr<Predicate>;
  // auto erase_not(std::unique_ptr<Predicate>, bool) -> std::unique_ptr<Predicate>;
  auto preprocess_select(SelectQuery &) -> std::unique_ptr<SelectStmt>;

 public:
  Preprocessor(ITranscationalStorageManager &, ResultPrinter &);

  auto preprocess(std::unique_ptr<Query>) -> std::unique_ptr<Statement>;
};
}  // namespace query_process_engine