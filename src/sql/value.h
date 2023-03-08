#pragma once

#include <string>

enum class ValueType {
  VT_STRING,
  VT_INT,
  VT_FLOAT,
  VT_DATE,
};

class Value {
public:
  virtual ValueType get_value_type() const = 0;
  virtual ~Value() {}
};

class Integer : public Value {
private:
  int number_;

public:
  explicit Integer(int num) : number_(num) {}
  ValueType get_value_type() const override {
    return ValueType::VT_INT;
  } 
  int get_number() const {
    return number_;
  }
};

class String : public Value {
private:
  std::string str_;

public:
  explicit String(const char* str) : str_(str + 1) { 
    str_.pop_back();
  }
  ValueType get_value_type() const override {
    return ValueType::VT_STRING;
  } 
  const std::string& get_str() const {
    return str_;
  }
};

class Float : public Value {
private:
  float floats_;
public:
  explicit Float(float fl) : floats_(fl) {}
  ValueType get_value_type() const override {
    return ValueType::VT_FLOAT;
  } 
  const float get_float() const {
    return floats_;
  }
};

// TODO: date class needs implementations
class Date : public Value {
public:
  ValueType get_value_type() const override {
    return ValueType::VT_DATE;
  } 
};