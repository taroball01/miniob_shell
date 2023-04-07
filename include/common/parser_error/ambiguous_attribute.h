#pragma once

#include <string>
#include "common/parser_error/parser_error_info.h"
#include "relation/attribute.h"
namespace query_process_engine {
class AmbiguousAttribute : public ParserErrorInfo {
private:
  std::string attr_;

public:
  explicit AmbiguousAttribute(const std::string& attr) : attr_(attr) {}

  auto get_raw_string(const std::string&) const -> std::string override {
    return attr_;
  } 

  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Ambiguous_Attribute;
  }

  auto get_details() const -> std::string override {
    static std::string prefix = "Ambiguous Attribute: ";
    return prefix + attr_;
  }
};

} // namespace query_process_engine