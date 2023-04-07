#pragma once

#include "common/parser_error/parser_error_info.h"
#include "relation/attribute.h"
namespace query_process_engine {

class UnknownAttribute : public ParserErrorInfo {
private:
  Attribute attribute_;

public:
  explicit UnknownAttribute(const Attribute& attribute) : attribute_(attribute) {}

  auto get_raw_string(const std::string& sql) const -> std::string override {
    return attribute_.to_string();
  } 
  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Unknown_Attribute;
  }
  auto get_details() const -> std::string override {
    static std::string prefix = "Unknown Attribute: ";
    return prefix + attribute_.to_string();
  }
};

} // namespace query_process_engine
