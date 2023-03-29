#pragma once 
#include "relation/value/value.h"

namespace query_process_engine {
class Date : public Value {
 public:
  using inner_type = const std::string &;

 private:
  int year_;
  short month_, day_;
  static constexpr int CORRECT_SIZE = 10;  // yyyy-mm-dd
 public:
  static auto is_valid_format(const std::string &date) -> bool;
  static auto parse(const std::string &date) -> std::tuple<bool, int, short, short>;
  static auto is_valid_date(int y, short m, short d) -> bool;
  static auto is_valid_date(const std::string &date) -> bool;

 public:
  explicit Date(const std::string &date);
  auto get_value_type() const -> ValueType override { return ValueType::VT_DATE; }
  auto get_year() -> int { return year_; }
  auto get_month() -> short { return month_; }
  auto get_day() -> short { return day_; }
  auto to_string() const -> std::string override;
  auto operator<(const Value &rhs) const -> bool override;
};
} // namespace query_process_engine