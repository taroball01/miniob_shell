#include "executor/operator.h"

namespace query_process_engine {
auto TableScanOperator::open() -> bool {
  p_next_id_ = ts_manager_.get_start_id(plan_.get_relation());
  return p_next_id_ != nullptr;
}

auto TableScanOperator::get_next(Tuple &tp) -> bool {
  if (p_next_id_ == nullptr) return false;
  tp.clear();
  tp = ts_manager_.get_tuple(*p_next_id_);
  p_next_id_ = ts_manager_.get_next_id(*p_next_id_);
  return true;
}

auto TableScanOperator::close() -> void { p_next_id_ = nullptr; }

}  // namespace query_process_engine