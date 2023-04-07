#pragma once

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include "sql/predicate/compare_op.h"

namespace query_process_engine {
enum class ValueType {
  VT_STRING,
  VT_INT,
  VT_FLOAT,
  VT_DATE,
  VT_INVALID,
};

auto valuetype_to_string(ValueType) -> std::string;

class Value {
 public:
  static auto evaluate(CompareOp op, const Value &l, const Value &r) -> bool;

 public:
  virtual ValueType get_value_type() const = 0;
  virtual ~Value() {}

  template <typename T>
  auto get_as() const -> typename T::inner_type {
    auto *ptr = dynamic_cast<const T *>(this);
    return ptr->get_data();
  }
  auto operator==(const Value &rhs) const -> bool { return !(*this < rhs) && !(rhs < *this); }
  auto operator>=(const Value &rhs) const -> bool { return !(*this < rhs); }
  auto operator>(const Value &rhs) const -> bool { return rhs < *this; }
  auto operator<=(const Value &rhs) const -> bool { return !(rhs < *this); }
  auto operator!=(const Value &rhs) const -> bool { return (*this < rhs) || (rhs < *this); }
  virtual auto operator<(const Value &rhs) const -> bool = 0;
  virtual auto to_string() const -> std::string = 0;
};

class Integer : public Value {
 public:
  using inner_type = int;

 private:
  int number_;

 public:
  explicit Integer(int num) : number_(num) {}
  ValueType get_value_type() const override { return ValueType::VT_INT; }
  int get_data() const { return number_; }

  auto operator<(const Value &rhs) const -> bool override {
    if (get_value_type() != rhs.get_value_type()) {
      throw std::logic_error("Integer::operator<");
    }
    auto &r_nm = dynamic_cast<const Integer &>(rhs);
    return get_data() < r_nm.get_data();
  }
  auto to_string() const -> std::string override { return std::to_string(number_); }
};

class String : public Value {
 public:
  using inner_type = const std::string &;

 private:
  std::string str_;

 public:
  explicit String(const char *str) : str_(str + 1) { str_.pop_back(); }
  ValueType get_value_type() const override { return ValueType::VT_STRING; }
  const std::string &get_data() const { return str_; }

  auto operator<(const Value &rhs) const -> bool override {
    if (get_value_type() != rhs.get_value_type()) {
      throw std::logic_error("String::operator<");
    }
    auto &r_st = dynamic_cast<const String &>(rhs);
    return get_data() < r_st.get_data();
  }
  auto to_string() const -> std::string override { return "'" + str_ + "'"; }
};

class Float : public Value {
 public:
  using inner_type = float;

 private:
  float floats_;

 public:
  explicit Float(float fl) : floats_(fl) {}
  ValueType get_value_type() const override { return ValueType::VT_FLOAT; }
  const float get_data() const { return floats_; }
  auto operator<(const Value &rhs) const -> bool override {
    if (get_value_type() != rhs.get_value_type()) {
      throw std::logic_error("Float::operator<");
    }
    auto &r_fl = dynamic_cast<const Float &>(rhs);
    return get_data() < r_fl.get_data();
  }
  auto to_string() const -> std::string override { return std::to_string(floats_); }
};


}  // namespace query_process_engine