
#include "executor/operator.h"
namespace query_process_engine {
auto FilterOperator::open() -> bool { return child_->open(); }

auto FilterOperator::get_next(Tuple &tp) -> bool {
  bool check = false;
  Tuple down;
  while (!check) {
    bool not_end = child_->get_next(down);
    if (!not_end) return false;
    check = plan_.get_condition().evaluate(down, child_->get_plan().get_output_schema());
  }
  tp = down;
  return true;
}

auto FilterOperator::close() -> void { child_->close(); }

}  // namespace query_process_engine