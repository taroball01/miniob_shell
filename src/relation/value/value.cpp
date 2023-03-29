#include "relation/value/value.h"

namespace query_process_engine {
auto Value::evaluate(CompareOp op, const Value &l, const Value &r) -> bool {
  switch (op) {
    case CompareOp::OP_EQ:
      return l == r;
    case CompareOp::OP_GE:
      return l >= r;
    case CompareOp::OP_GT:
      return l > r;
    case CompareOp::OP_LE:
      return l <= r;
    case CompareOp::OP_LT:
      return l < r;
    case CompareOp::OP_NE:
      return l != r;
    default:
      throw std::logic_error("Value::evaluate");
  }
}


} // namespace query_process_engine