#include "executor/operator.h"
#include "relation/tuple.h"
#include "relation/value/value.h"
namespace query_process_engine {

auto DeleteOperator::open() -> bool { return child_->open(); }

auto DeleteOperator::get_next(Tuple &tp) -> bool {
  if (cnt_ != 0) {
    return false;
  }
  ++cnt_;
  
  int succ = 0, fail = 0;
  Tuple fetched;
  while (child_->get_next(fetched)) {
    bool del_res = ts_manager_.delete_tuple(*fetched.get_id());
    succ += del_res;
    fail += !del_res;
    fetched.clear();
  }
  tp = make_tuple(Integer(succ), Integer(fail));
  return true;
}

auto DeleteOperator::close() -> void {
  child_->close();
}
} // namespace query_process_engine
