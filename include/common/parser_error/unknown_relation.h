#pragma once

#include "common/parser_error/parser_error_info.h"
namespace query_process_engine {

class UnknownRelation : public ParserErrorInfo {
private:
  std::string relation_;

public:
  explicit UnknownRelation(const std::string& rel) : relation_(rel) {}
  auto get_raw_string(const std::string&) const -> std::string override {
    return relation_;
  }
  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Unknown_Relation;
  }
  auto get_details() const -> std::string override {
    static std::string prefix = "Unknown Relation: ";
    return prefix + relation_;
  }
};

} // namespace query_process_engine
