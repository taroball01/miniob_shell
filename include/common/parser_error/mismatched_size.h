#pragma once

#include <string>
#include "common/parser_error/parser_error_info.h"
#include "relation/attribute.h"
namespace query_process_engine {

class MismatchedSize : public ParserErrorInfo {
private:
  int val_size_;
  int rel_size_;

public:
  explicit MismatchedSize(int vs, int rs) : val_size_(vs), rel_size_(rs) {}

  auto get_raw_string(const std::string& sql) const -> std::string override {
    return sql;
  } 
  auto get_error_type() const -> ParserErrorType override {
    return ParserErrorType::Mismatched_Size;
  }
  auto get_details() const -> std::string override {
    static std::string prefix = "Mismatched size in relation : ";
    return prefix + std::to_string(val_size_) + " <> " + std::to_string(rel_size_);
  }
};

} // namespace query_process_engine
