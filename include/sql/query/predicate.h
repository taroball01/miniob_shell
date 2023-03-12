#pragma once 

#include <memory>
#include <string>
#include "sql/query/attribute.h"
#include "sql/query/value.h"

enum class PredicateType {
  CNT_NOT,
  CNT_AND,
  CNT_OR,
  CNT_LEAF,
};

enum class CompareOp {
  OP_EQ,
  OP_LE,
  OP_NE,
  OP_LT,
  OP_GE,
  OP_GT,
};

enum class OperandType {
  COT_ATTR,
  COT_VAL,
};

class Predicate {
public:
  virtual PredicateType get_node_type() const = 0;
  virtual ~Predicate() {}
  template <typename T>
  auto get_as() const -> const T& {
    auto* ptr = dynamic_cast<const T*>(this);
    assert(nullptr != ptr);
    return *ptr;
  }
};

class PredicateNot : public Predicate {
private:
  std::unique_ptr<Predicate> child_;
public:
  explicit PredicateNot(std::unique_ptr<Predicate>&& ch) : child_(std::move(ch)) {}
  PredicateType get_node_type() const override {
    return PredicateType::CNT_NOT;
  }
  auto get_child() const -> const Predicate& {
    return *child_;
  }
};

class PredicateAnd : public Predicate {
private:
  std::unique_ptr<Predicate> lchild_, rchild_;
public:
  PredicateAnd(std::unique_ptr<Predicate>&& lch, std::unique_ptr<Predicate>&& rch) : lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  PredicateType get_node_type() const override {
    return PredicateType::CNT_AND;
  }
  auto get_lchild() const -> const Predicate& {
    return *lchild_;
  }
  auto get_rchild() const -> const Predicate& {
    return *rchild_;
  }
};

class PredicateOr : public Predicate {
private:
  std::unique_ptr<Predicate> lchild_, rchild_;
public:
  PredicateOr(std::unique_ptr<Predicate>&& lch, std::unique_ptr<Predicate>&& rch) : lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  PredicateType get_node_type() const override {
    return PredicateType::CNT_OR;
  }
  auto get_lchild() const -> const Predicate& {
    return *lchild_;
  }

  auto get_rchild() const -> const Predicate& {
    return *rchild_;
  }
};

class Operand {
public:
  virtual OperandType get_operand_type() const = 0;
  virtual ~Operand() {}
  template <typename T>
  auto get_as() const -> const T& {
    auto* ptr = dynamic_cast<const T*>(this);
    assert(ptr != nullptr);
    return *ptr;
  }
};

class OperandAttr : public Operand {
private:
  Attribute attr_;
public:
  explicit OperandAttr(Attribute&& attr) : attr_(std::move(attr)) {}
  OperandType get_operand_type() const override {
    return OperandType::COT_ATTR;
  }
  auto get_attr() const -> const Attribute& {
    return attr_;
  }
};

class OperandVal : public Operand {
private:
  std::unique_ptr<Value> val_;
public:
  explicit OperandVal(std::unique_ptr<Value>&& val) : val_(std::move(val)) {}
  OperandType get_operand_type() const override {
    return OperandType::COT_VAL;
  }
  auto get_value() const -> const Value& {
    return *val_;
  }
};

class PredicateLeaf : public Predicate {
private:
  CompareOp op_;
  std::unique_ptr<Operand> lchild_, rchild_;
public:
  PredicateLeaf(CompareOp op, std::unique_ptr<Operand>&& lch, std::unique_ptr<Operand>&& rch)
    : op_(op), lchild_(std::move(lch)), rchild_(std::move(rch)) {}

  PredicateType get_node_type() const override {
    return PredicateType::CNT_LEAF;
  }
  auto get_op() const -> CompareOp {
    return op_;
  }
  auto get_lchild() const -> const Operand& {
    return *lchild_;
  }
  auto get_rchild() const -> const Operand& {
    return *rchild_;
  }
};