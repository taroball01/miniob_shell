#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include "relation/value/value.h"
#include "sql/predicate/operand.h"
namespace query_process_engine {

class Tuple;
class Schema;

enum class PredicateType { CNT_NOT, CNT_AND, CNT_OR, CNT_LEAF, CNT_RAW };

class Predicate {
 public:
  virtual PredicateType get_node_type() const = 0;
  virtual auto evaluate(const Tuple &tp, const Schema &sch) const -> bool = 0;
  virtual ~Predicate() {}
  // For test
  virtual auto operator==(const Predicate& rhs) const -> bool = 0;
  template <typename T>
  auto get_as() -> T & {
    auto *ptr = dynamic_cast<T *>(this);
    assert(nullptr != ptr);
    return *ptr;
  }
};

class PredicateNot : public Predicate {
 private:
  std::unique_ptr<Predicate> child_;

 public:
  explicit PredicateNot(std::unique_ptr<Predicate> &&ch) : child_(std::move(ch)) {}
  PredicateType get_node_type() const override { return PredicateType::CNT_NOT; }
  auto evaluate(const Tuple &tp, const Schema &sch) const -> bool override { return !child_->evaluate(tp, sch); }
  
  auto operator==(const Predicate& rhs) const -> bool override {
    bool same = rhs.get_node_type() == get_node_type();
    return same && *child_ == *dynamic_cast<const PredicateNot&>(rhs).child_;
  }
  auto get_child() -> Predicate & { return *child_; }
  // This call will transfer ownership of child_;
  auto transfer_child() -> std::unique_ptr<Predicate> { return std::move(child_); }

  auto set_child(std::unique_ptr<Predicate> ch) -> void { child_ = std::move(ch); }
};

class PredicateAnd : public Predicate {
 private:
  std::unique_ptr<Predicate> lchild_, rchild_;

 public:
  PredicateAnd(std::unique_ptr<Predicate> &&lch, std::unique_ptr<Predicate> &&rch)
      : lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  PredicateType get_node_type() const override { return PredicateType::CNT_AND; }
  auto evaluate(const Tuple &tp, const Schema &sch) const -> bool override {
    return lchild_->evaluate(tp, sch) && rchild_->evaluate(tp, sch);
  }
  
  auto operator==(const Predicate& rhs) const -> bool override{
    bool same = get_node_type() == rhs.get_node_type();
    if (!same) {
      return false;
    } 
    auto& and_pred = dynamic_cast<const PredicateAnd&>(rhs);
    return *and_pred.lchild_ == *lchild_ && *rchild_ == *and_pred.rchild_; 
  } 

  auto get_lchild() -> Predicate & { return *lchild_; }
  auto get_rchild() -> Predicate & { return *rchild_; }
  auto transfer_lchild() -> std::unique_ptr<Predicate> { return std::move(lchild_); }
  auto transfer_rchild() -> std::unique_ptr<Predicate> { return std::move(rchild_); }
  auto set_lchild(std::unique_ptr<Predicate> ch) -> void { lchild_ = std::move(ch); }
  auto set_rchild(std::unique_ptr<Predicate> ch) -> void { rchild_ = std::move(ch); }
};

class PredicateOr : public Predicate {
 private:
  std::unique_ptr<Predicate> lchild_, rchild_;

 public:
  PredicateOr(std::unique_ptr<Predicate> &&lch, std::unique_ptr<Predicate> &&rch)
      : lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  PredicateType get_node_type() const override { return PredicateType::CNT_OR; }
  auto evaluate(const Tuple &tp, const Schema &sch) const -> bool override {
    return lchild_->evaluate(tp, sch) || rchild_->evaluate(tp, sch);
  }
  auto operator==(const Predicate& rhs) const -> bool override{
    bool same = get_node_type() == rhs.get_node_type();
    if (!same) {
      return false;
    } 
    auto& or_pred = dynamic_cast<const PredicateOr&>(rhs);
    return *or_pred.lchild_ == *lchild_ && *rchild_ == *or_pred.rchild_; 
  } 

  auto get_lchild() -> Predicate & { return *lchild_; }
  auto get_rchild() -> Predicate & { return *rchild_; }
  auto transfer_lchild() -> std::unique_ptr<Predicate> { return std::move(lchild_); }
  auto transfer_rchild() -> std::unique_ptr<Predicate> { return std::move(rchild_); }
  auto set_lchild(std::unique_ptr<Predicate> ch) -> void { lchild_ = std::move(ch); }
  auto set_rchild(std::unique_ptr<Predicate> ch) -> void { rchild_ = std::move(ch); }
};

class PredicateLeaf : public Predicate {
 private:
  CompareOp op_;
  std::unique_ptr<Operand> lchild_, rchild_;

 private:
  static auto op_to_string(CompareOp op) -> std::string {
    switch (op) {
      case CompareOp::OP_EQ:
        return "=";
      case CompareOp::OP_GE:
        return ">=";
      case CompareOp::OP_GT:
        return ">";
      case CompareOp::OP_LE:
        return "<=";
      case CompareOp::OP_LT:
        return "<";
      case CompareOp::OP_NE:
        return "!=";
      default:
        throw std::logic_error("PredicateLeaft::op_to_string");
    }
  }

 public:
  PredicateLeaf(CompareOp op, std::unique_ptr<Operand> &&lch, std::unique_ptr<Operand> &&rch)
      : op_(op), lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  auto get_node_type() const -> PredicateType override { return PredicateType::CNT_LEAF; }
  
  auto evaluate(const Tuple &tp, const Schema &sch) const -> bool override {
    return Value::evaluate(op_, lchild_->resolve_value(tp, sch), rchild_->resolve_value(tp, sch));
  }

  auto operator==(const Predicate& rhs) const -> bool override{
    bool same = get_node_type() == rhs.get_node_type();
    if (!same) {
      return false;
    } 
    auto& leaf_pred = dynamic_cast<const PredicateLeaf&>(rhs);
    return op_ == leaf_pred.op_ && *leaf_pred.lchild_ == *lchild_ && *rchild_ == *leaf_pred.rchild_; 
  } 

  auto get_op() const -> CompareOp { return op_; }
  auto get_lchild() -> Operand & { return *lchild_; }
  auto get_rchild() -> Operand & { return *rchild_; }
  auto to_string() -> std::string { return lchild_->to_string() + op_to_string(op_) + rchild_->to_string(); }

  auto check_simple() -> bool {
    return lchild_->get_operand_type() == OperandType::COT_VAL && rchild_->get_operand_type() == OperandType::COT_VAL;
  }
};

class PredicateRaw : public Predicate {
 private:
  bool value_;

 public:
  explicit PredicateRaw(bool v) : value_(v) {}
  PredicateType get_node_type() const override { return PredicateType::CNT_RAW; }
  auto evaluate(const Tuple &tp, const Schema &sch) const -> bool override { return value_; }
  
  auto operator==(const Predicate& rhs) const -> bool override {
    bool same = get_node_type() == rhs.get_node_type();
    if (!same) {
      return false;
    } 
    auto raw_rhs = dynamic_cast<const PredicateRaw&>(rhs);
    return raw_rhs.value_ == value_;
  }

  auto get_value() const -> bool { return value_; }
  auto set_value(bool new_v) -> void { value_ = new_v; }
};
}  // namespace query_process_engine