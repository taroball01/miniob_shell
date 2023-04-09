
#include "executor/operator.h"
#include "relation/tuple.h"
#include "relation/value/value.h"
namespace query_process_engine {

auto InsertOperator::open() -> bool { return !ts_manager_.get_relation(plan_.get_relation_name()).empty(); }

auto InsertOperator::get_next(Tuple &tp) -> bool {
  if (cnt_ != 0) {
    return false;
  }
  ++cnt_;
  auto &relation = plan_.get_relation_name();
  auto &arr = plan_.get_value_arr();
  if (ts_manager_.insert_tuple(relation, arr)) {
    tp = make_tuple(Integer(1));
    return true;
  }
  tp = make_tuple(Integer(0));
  return true;
}

auto InsertOperator::close() -> void {}

}  // namespace query_process_engine
