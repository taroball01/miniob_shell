#pragma once

#include "common/parser_error/parser_error_info.h"
namespace query_process_engine {

class UnknownCharacter : public ParserErrorInfo {
private:
  char unknown_;
  int line_;
  int col_;

public:
  UnknownCharacter(char ch, int line, int col) : unknown_(ch), line_(line), col_(col) {}
  auto get_position() -> std::pair<int,int> override {
    return std::make_pair(line_, col_);
  }

  auto get_raw_string(const std::string& sql) const -> std::string override {
    return match_raw(sql, line_);
  }

  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Unknown_Character;
  }
  auto get_details() const -> std::string override {
    static std::string prefix = "Unknown Character: ";
    return prefix + unknown_;
  }
};
} // namespace query_process_engine