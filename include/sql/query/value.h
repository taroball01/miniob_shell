#pragma once

#include <string>

enum class ValueType {
  VT_STRING,
  VT_INT,
  VT_FLOAT,
  VT_DATE,
};
class Integer;
class Float;
class Value {
public:
  virtual ValueType get_value_type() const = 0;
  virtual ~Value() {}

  template <typename T>
  auto get_as() const -> typename T::inner_type {
    auto* ptr = dynamic_cast<const T*>(this);
    return ptr->get_data();
  }
};

class Integer : public Value {
public:
  using inner_type = int;
private:
  int number_;
public:
  explicit Integer(int num) : number_(num) {}
  ValueType get_value_type() const override {
    return ValueType::VT_INT;
  } 
  int get_data() const {
    return number_;
  }
};

class String : public Value {
public:
  using inner_type = const std::string&;
private:
  std::string str_;
public:
  explicit String(const char* str) : str_(str + 1) { 
    str_.pop_back();
  }
  ValueType get_value_type() const override {
    return ValueType::VT_STRING;
  } 
  const std::string& get_data() const {
    return str_;
  }
};

class Float : public Value {
public:
  using inner_type = float;
private:
  float floats_;
public:
  explicit Float(float fl) : floats_(fl) {}
  ValueType get_value_type() const override {
    return ValueType::VT_FLOAT;
  } 
  const float get_data() const {
    return floats_;
  }
};
