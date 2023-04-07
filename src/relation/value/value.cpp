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

auto valuetype_to_string(ValueType vt) -> std::string {
  switch (vt) {
    case ValueType::VT_STRING: return "String";
    case ValueType::VT_DATE: return "Date";
    case ValueType::VT_FLOAT: return "Float";
    case ValueType::VT_INT: return "Integer";
    default: return "Invalid";
  }
}
} // namespace query_process_engine