#pragma once

#include "common/parser_error/parser_error_info.h"
#include "relation/value/value.h"
namespace query_process_engine {
class MismatchedType : public ParserErrorInfo {
private:
  std::string part_;
  ValueType l_type_;
  ValueType r_type_;

public:
  MismatchedType(const std::string& pt, ValueType lt, ValueType rt) 
  : part_(pt), l_type_(lt), r_type_(rt) {}

  auto get_raw_string(const std::string&) const -> std::string override {
    return part_;
  }
  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Mismatched_Type;
  }

  auto get_details() const -> std::string override {
    static std::string prefix = "Mismatched type: ";
    std::string lts = valuetype_to_string(l_type_);
    std::string rts = valuetype_to_string(r_type_);

    return prefix + lts + " <> " + rts;
  }
};
}