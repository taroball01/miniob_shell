#include "executor/operator.h"

namespace query_process_engine {
auto ProjectionOperator::open() -> bool { return child_->open(); }

auto ProjectionOperator::get_next(Tuple &tp) -> bool {
  Tuple to_pro_tp;
  bool check = child_->get_next(to_pro_tp);
  if (!check) {
    return false;
  }
  tp.clear();
  auto &to_pro_sch = child_->get_plan().get_output_schema();
  for (auto &item : plan_.get_output_schema().get_schema_array()) {
    auto &rel = item.relation_;
    auto &attr = item.attribute_;
    int id = to_pro_sch.get_attribute_id(rel, attr);
    tp.append_back(to_pro_tp.get_tuple_array().at(id));
  }
  return true;
}

auto ProjectionOperator::close() -> void { child_->close(); }
}  // namespace query_process_engine
