#include "relation/value/date.h"

namespace query_process_engine {

auto Date::is_valid_format(const std::string &date) -> bool {
  auto is_digit = [](char c) { return c >= '0' && c <= '9'; };
  bool valid = true;

  valid &= date.size() == CORRECT_SIZE;

  for (int i = 0; i < CORRECT_SIZE; ++i) {
    if (i == 4 || i == 7) {
      valid &= date[i] == '-';
    } else {
      valid &= is_digit(date[i]);
    }
  }
  return valid;
}

auto Date::parse(const std::string &date) -> std::tuple<bool, int, short, short> {
  int year = 0;
  short month = 0, day = 0;
  bool valid = is_valid_format(date);
  auto char2num = [](char c) { return c - '0'; };
  if (valid) {
    year = char2num(date[0]) * 1000 + char2num(date[1]) * 100 + char2num(date[2]) * 10 + char2num(date[3]);
    month = char2num(date[5]) * 10 + char2num(date[6]);
    day = char2num(date[8]) * 10 + char2num(date[9]);
  }
  return std::make_tuple(valid, year, month, day);
}

auto Date::is_valid_date(int y, short m, short d) -> bool {
  bool valid = y >= 0 && m > 0 && d > 0;
  if (!valid) {
    return false;
  }
  // check for each month
  valid &= m <= 12;
  auto is_leap_year = [](int y) { return y % 400 == 0 || (y % 4 == 0 && y % 100 != 0); };
  switch (m) {
    case 4:
    case 6:
    case 9:
    case 11: {
      valid &= d <= 30;
      break;
    }
    case 2: {
      valid &= d <= (is_leap_year(y) ? 29 : 28);
      break;
    }
    default: {
      valid &= d <= 31;
      break;
    }
  }
  return valid;
}

auto Date::is_valid_date(const std::string &date) -> bool {
  auto [valid, y, m, d] = parse(date);
  if (!valid) {
    return false;
  }
  return is_valid_date(y, m, d);
}

Date::Date(const std::string &date) {
  auto [v, y, m, d] = parse(date);
  year_ = y;
  month_ = m;
  day_ = d;
  if (!v || !is_valid_date(y, m, d)) {
    throw std::logic_error("Date::Date:" + date + " is not a valid date");
  }
}

auto Date::to_string() const -> std::string {
  std::ostringstream os;
  os << std::setw(4) << std::setfill('0') << year_ << '-' << std::setw(2) << std::setfill('0') << month_ << '-'
      << std::setw(2) << std::setfill('0') << day_;
  return os.str();
}

auto Date::operator<(const Value &rhs) const -> bool  {
  if (get_value_type() != rhs.get_value_type()) {
    throw std::logic_error("Integer::operator<");
  }
  auto &r_dt = dynamic_cast<const Date &>(rhs);
  return (year_ == r_dt.year_) ? (month_ == r_dt.month_ ? day_ < r_dt.day_ : month_ < r_dt.month_)
                                : year_ < r_dt.year_;
}
} // namespace query_process_engine
