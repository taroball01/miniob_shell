#include "executor/operator.h"

namespace query_process_engine {
NestLoopJoinOperator::NestLoopJoinOperator(BinaryJoinPlanNode &plan, std::unique_ptr<PhysicalOperator> lchild,
                                           std::unique_ptr<PhysicalOperator> rchild)
    : plan_(plan), lchild_(std::move(lchild)), rchild_(std::move(rchild)), l_tp_() {}

auto NestLoopJoinOperator::open() -> bool { return lchild_->open() && rchild_->open() && lchild_->get_next(l_tp_); }

auto NestLoopJoinOperator::get_next(Tuple &tp) -> bool {
  Tuple r_tp;

  bool inner_end = !rchild_->get_next(r_tp);
  while (inner_end) {
    bool outer_end = !lchild_->get_next(l_tp_);
    if (outer_end) {
      return false;
    }
    inner_end = !rchild_->open() || !rchild_->get_next(r_tp);
  }
  tp.clear();

  int l_size = l_tp_.get_tuple_array().size();
  for (int i = 0; i < l_size; ++i) {
    tp.append_back(l_tp_.get_tuple_array().at(i));
  }

  int r_size = r_tp.get_tuple_array().size();
  for (int i = 0; i < r_size; ++i) {
    tp.append_back(r_tp.get_tuple_array().at(i));
  }
  return true;
}

auto NestLoopJoinOperator::close() -> void {
  lchild_->close();
  rchild_->close();
}
}  // namespace query_process_engine