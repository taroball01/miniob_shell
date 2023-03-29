#pragma once
#include <memory>
#include <cassert>
#include "relation/attribute.h"
#include "relation/schema.h"
#include "relation/tuple.h"
#include "relation/value/value.h"

namespace query_process_engine {
enum class OperandType {
  COT_ATTR,
  COT_VAL,
};

class Operand {
 public:
  virtual OperandType get_operand_type() const = 0;
  virtual ~Operand() {}
  virtual auto to_string() const -> std::string = 0;
  virtual auto resolve_value(const Tuple &tp, const Schema &sch) const -> const Value & = 0;
  virtual auto operator==(const Operand& rhs) const -> bool = 0;
  template <typename T>
  auto get_as() -> T & {
    auto *ptr = dynamic_cast<T *>(this);
    assert(ptr != nullptr);
    return *ptr;
  }
};

class OperandAttr : public Operand {
 private:
  Attribute attr_;

 public:
  explicit OperandAttr(Attribute &&attr) : attr_(std::move(attr)) {}
  OperandType get_operand_type() const override { return OperandType::COT_ATTR; }
  auto to_string() const -> std::string override { return attr_.to_string(); }
  auto operator==(const Operand& rhs) const -> bool override {
    bool same = get_operand_type() == rhs.get_operand_type();
    if (!same) {
      return false;
    }
    auto& opnd = dynamic_cast<const OperandAttr&>(rhs);
    return opnd.attr_ == attr_;
  }
  auto get_attr() -> Attribute & { return attr_; }
  auto resolve_value(const Tuple &tp, const Schema &sch) const -> const Value & override {
    int id = sch.get_attribute_id(attr_.get_relation(), attr_.get_attribute());
    return *(tp.get_tuple_array().at(id));
  }
};

class OperandVal : public Operand {
 private:
  std::unique_ptr<Value> val_;

 public:
  explicit OperandVal(std::unique_ptr<Value> &&val) : val_(std::move(val)) {}
  OperandType get_operand_type() const override { return OperandType::COT_VAL; }
  auto to_string() const -> std::string override { return val_->to_string(); }
  auto operator==(const Operand& rhs) const -> bool override {
    bool same = get_operand_type() == rhs.get_operand_type();
    if (!same) {
      return false;
    }
    auto& opnd = dynamic_cast<const OperandVal&>(rhs);
    // must compare Value& instead of unique_ptr<Val>
    return opnd.val_->get_value_type() == val_->get_value_type() && *opnd.val_ == *val_;
  }
  
  auto get_value() const -> const Value & { return *val_; }
  auto set_value(std::unique_ptr<Value> val) -> void { val_ = std::move(val); }
  auto resolve_value(const Tuple &, const Schema &) const -> const Value & override { return *val_; }
};
}  // namespace query_process_engine