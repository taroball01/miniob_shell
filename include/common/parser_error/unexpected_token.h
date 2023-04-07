#pragma once

#include <exception>
#include "common/parser_error/parser_error_info.h"
namespace query_process_engine {

class UnexpectedToken : public ParserErrorInfo {
private:
  std::string details_;
  int line_;
  int col_;

public:
  UnexpectedToken(const std::string& details, int line, int col) : details_(details), line_(line), col_(col) {}
  auto get_position() -> std::pair<int,int> override {
    return std::make_pair(line_, col_);
  }
  
  auto get_raw_string(const std::string& sql) const -> std::string override {
    return match_raw(sql, line_);
  } 

  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Unexpected_Token;
  }
  auto get_details() const -> std::string override {
    return details_;
  }
};
} // namespace query_process_engine