#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>
#include "preprocess/preprocessor.h"
#include "relation/value/date.h"
#include "relation/value/value.h"
#include "sql/predicate/operand.h"
#include "sql/predicate/predicate.h"

namespace query_process_engine {
Preprocessor::Preprocessor(ITranscationalStorageManager &tsm, ResultPrinter &printer)
    : ts_manager_(tsm), printer_(printer) {}

auto Preprocessor::preprocess(std::unique_ptr<Query> query) -> std::unique_ptr<Statement> {
  switch (query->get_sql_type()) {
    case SqlType::Select:
      return preprocess_select(dynamic_cast<SelectQuery &>(*query));
    default:
      return nullptr;
  }
}

auto Preprocessor::preprocess_select(SelectQuery &query) -> std::unique_ptr<SelectStmt> {
  Schema sch;
  if (!resolve_relations(query.get_relations(), sch)) {
    return nullptr;
  }
  if (!resolve_attributes(query.get_attributes(), sch)) {
    return nullptr;
  }
  printer_.set_attributes(query.get_attributes());

  if (!query.is_conditions_exist()) {
    query.set_condition(std::make_unique<PredicateRaw>(true));
  }
  if (!resolve_predicate_leaves(query.get_conditions(), sch)) {
    return nullptr;
  }
  // we assure that parse success here
  auto ptr = pre_compute_simple_leaves(query.transfer_conditions());

  return std::make_unique<SelectStmt>(sch, query.get_attributes(), std::move(ptr));
}

auto Preprocessor::resolve_relations(const std::vector<std::string> &relations, Schema &schema) -> bool {
  for (auto &rel : relations) {
    auto sch_arr = ts_manager_.get_relation(rel);
    if (sch_arr.empty()) {
      throw std::runtime_error("unknown relation:" + rel);
      return false;
    }
    for (auto &item : sch_arr) {
      schema.append_schema(item);
    }
  }
  return true;
}

auto Preprocessor::resolve_attribute_item(Attribute &attr, const Schema &schema) -> bool {
  auto &attr_name = attr.get_attribute();
  auto &rel_name = attr.get_relation();
  if (rel_name.empty()) {
    // find relation
    int rel_num = schema.count_relation(attr_name);
    if (1 != rel_num) {
      throw std::runtime_error(rel_num > 1 ? ("ambiguous attribute: " + attr_name)
                                           : ("unknown attribute: " + attr_name));
      return false;
    } else {
      attr.set_relation(schema.get_relation(attr_name));
    }
  } else {
    // check whether exists
    if (!schema.is_attribute_exists(rel_name, attr_name)) {
      throw std::runtime_error("unknown attribute: " + rel_name + "." + attr_name);
      return false;
    }
  }
  return true;
}

auto Preprocessor::resolve_attributes(std::vector<Attribute> &attributes, const Schema &schema) -> bool {
  // first scan the attributes to find * and rel.*
  bool all_in_one = false;
  for (auto &attr : attributes) {
    if (attr.get_attribute() == "*") {
      auto &rel = attr.get_relation();
      if (rel.empty()) {
        all_in_one = true;
        break;
      }
      throw std::logic_error("Preprocessor::resolve_attributes");
    }
  }
  // check all in one
  if (all_in_one) {
    if (attributes.size() > 1) {
      printer_.output_warn("Found * in attribute list, other parts ignored.");
    }
    attributes.clear();
    auto &sch_arr = schema.get_schema_array();
    std::transform(sch_arr.begin(), sch_arr.end(), std::back_inserter(attributes), [](const SchemaItem &it) {
      return Attribute{it.attribute_, it.relation_};
    });
    return true;
  }
  // then check all_attributes_relations
  for (auto &attr : attributes) {
    if (!resolve_attribute_item(attr, schema)) {
      return false;
    }
  }
  return true;
}

auto Preprocessor::resolve_operand_type(Operand &oprd, const Schema &schema) -> ValueType {
  if (oprd.get_operand_type() == OperandType::COT_ATTR) {
    auto &oprd_attr = dynamic_cast<OperandAttr &>(oprd);
    auto &attr = oprd_attr.get_attr();
    bool flag = resolve_attribute_item(attr, schema);
    if (!flag) {
      return ValueType::VT_INVALID;
    } else {
      return schema.get_attribute_schema(attr.get_relation(), attr.get_attribute()).type_;
    }
  } else {
    auto &oprd_val = dynamic_cast<OperandVal &>(oprd);
    return oprd_val.get_value().get_value_type();
  }
}

auto Preprocessor::is_valid_compare(Operand &lhs, Operand &rhs, ValueType l_vt, ValueType r_vt) -> bool {
  if (l_vt == ValueType::VT_INVALID || r_vt == ValueType::VT_INVALID) {
    return false;
  }
  bool is_l_attr = lhs.get_operand_type() == OperandType::COT_ATTR;
  bool is_r_attr = rhs.get_operand_type() == OperandType::COT_ATTR;
  if (is_l_attr ^ is_r_attr) {
    auto attr_vt = is_l_attr ? l_vt : r_vt;
    auto val_vt = is_l_attr ? r_vt : l_vt;
    auto &r_val = is_l_attr ? rhs : lhs;
    if (attr_vt == ValueType::VT_DATE && val_vt == ValueType::VT_STRING) {
      auto &val = dynamic_cast<OperandVal &>(r_val);
      auto &str = val.get_value().get_as<String>();
      if (Date::is_valid_date(str)) {
        val.set_value(std::make_unique<Date>(str));
        return true;
      } else {
        return false;
      }
    }
  }
  return l_vt == r_vt;
}

auto Preprocessor::resolve_predicate_leaves(Predicate &conditions, const Schema &schema) -> bool {
  switch (conditions.get_node_type()) {
    case PredicateType::CNT_LEAF: {
      auto &leaf = dynamic_cast<PredicateLeaf &>(conditions);
      auto &lchild = leaf.get_lchild();
      auto &rchild = leaf.get_rchild();
      ValueType l_vt = resolve_operand_type(lchild, schema);
      ValueType r_vt = resolve_operand_type(rchild, schema);
      if (l_vt == ValueType::VT_INVALID || r_vt == ValueType::VT_INVALID) {
        return false;
      }

      if (!is_valid_compare(lchild, rchild, l_vt, r_vt)) {
        throw std::runtime_error("mismatched types in: " + leaf.to_string());
        return false;
      }
      return true;
    }
    case PredicateType::CNT_AND: {
      auto &pred_and = dynamic_cast<PredicateAnd &>(conditions);
      return resolve_predicate_leaves(pred_and.get_lchild(), schema) &&
             resolve_predicate_leaves(pred_and.get_rchild(), schema);
    }
    case PredicateType::CNT_OR: {
      auto &pred_or = dynamic_cast<PredicateOr &>(conditions);
      return resolve_predicate_leaves(pred_or.get_lchild(), schema) &&
             resolve_predicate_leaves(pred_or.get_rchild(), schema);
    }
    case PredicateType::CNT_NOT: {
      auto &pred_not = dynamic_cast<PredicateNot &>(conditions);
      return resolve_predicate_leaves(pred_not.get_child(), schema);
    }
    case PredicateType::CNT_RAW: {
      return true;
    }
    default: {
      // do not allow this type of node
      throw std::logic_error("Preprocessor::resolve_predicate_leaves");
    }
  }
}

auto Preprocessor::pre_compute_simple_leaves(std::unique_ptr<Predicate> condition) -> std::unique_ptr<Predicate> {
  // we need other support
  switch (condition->get_node_type()) {
    case PredicateType::CNT_LEAF: {
      auto &leaf = dynamic_cast<PredicateLeaf &>(*condition);
      if (leaf.check_simple()) {
        auto &lval = leaf.get_lchild().get_as<OperandVal>().get_value();
        auto &rval = leaf.get_rchild().get_as<OperandVal>().get_value();
        return std::make_unique<PredicateRaw>(Value::evaluate(leaf.get_op(), lval, rval));
      } else {
        return condition;
      }
    }
    case PredicateType::CNT_AND: {
      auto &and_pred = dynamic_cast<PredicateAnd &>(*condition);
      auto lch = pre_compute_simple_leaves(and_pred.transfer_lchild());
      auto rch = pre_compute_simple_leaves(and_pred.transfer_rchild());

      if (lch->get_node_type() == PredicateType::CNT_RAW) {
        auto &raw = dynamic_cast<PredicateRaw &>(*lch);
        // true then return another, false then return self
        return raw.get_value() ? std::move(rch) : std::move(lch);
      } else if (rch->get_node_type() == PredicateType::CNT_RAW) {
        auto &raw = dynamic_cast<PredicateRaw &>(*rch);
        return raw.get_value() ? std::move(lch) : std::move(rch);
      } else {
        and_pred.set_lchild(std::move(lch));
        and_pred.set_rchild(std::move(rch));
        return condition;
      }
    }
    case PredicateType::CNT_OR: {
      auto &or_pred = dynamic_cast<PredicateOr &>(*condition);
      auto lch = pre_compute_simple_leaves(or_pred.transfer_lchild());
      auto rch = pre_compute_simple_leaves(or_pred.transfer_rchild());

      if (lch->get_node_type() == PredicateType::CNT_RAW) {
        // true return self
        auto &raw = dynamic_cast<PredicateRaw &>(*lch);
        return raw.get_value() ? std::move(lch) : std::move(rch);
      } else if (rch->get_node_type() == PredicateType::CNT_RAW) {
        auto &raw = dynamic_cast<PredicateRaw &>(*rch);
        return raw.get_value() ? std::move(rch) : std::move(lch);
      } else {
        or_pred.set_lchild(std::move(lch));
        or_pred.set_rchild(std::move(rch));
        return condition;
      }
    }
    case PredicateType::CNT_NOT: {
      auto &not_pred = dynamic_cast<PredicateNot &>(*condition);
      auto ch = pre_compute_simple_leaves(not_pred.transfer_child());
      if (ch->get_node_type() == PredicateType::CNT_RAW) {
        auto &raw = dynamic_cast<PredicateRaw &>(*ch);
        raw.set_value(!raw.get_value());
        // simply return !ch
        return ch;
      } else {
        not_pred.set_child(std::move(ch));
        return condition;
      }
    }
    case PredicateType::CNT_RAW: {
      return condition;
    }
    default: {
      throw std::logic_error("Preprocessor::pre_compute_simple_leaves");
    }
  }
}

}  // namespace query_process_engine