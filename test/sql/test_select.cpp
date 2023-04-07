#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include "common/parser_error/parser_error_info.h"
#include "sql/parser.h"
#include "sql/predicate/predicate.h"
#include "sql/query/select.h"

using namespace query_process_engine;

bool check_stacks_empty(ParserContext &context) {
  return context.str_stack_.empty() && context.attr_stack_.empty() && context.value_stack_.empty() &&
         context.operand_stack_.empty() && context.pred_stack_.empty();
}

BOOST_AUTO_TEST_CASE(test_simple_select) {
  std::string sql = "select name from student;";
  ParserContext pc;
  int result = sql_parse(sql, pc);
  BOOST_CHECK(result == 0);
  BOOST_CHECK_MESSAGE(check_stacks_empty(pc), "stacks should be empty");
  auto *query = pc.get_query<SelectQuery>();
  BOOST_CHECK(nullptr != query);
  auto &attributes = query->get_attributes();
  BOOST_CHECK_EQUAL(attributes.size(), 1);
  BOOST_CHECK_EQUAL(attributes[0].get_attribute(), "name");
  BOOST_CHECK_EQUAL(attributes[0].get_relation(), "");

  auto &relations = query->get_relations();
  BOOST_CHECK_EQUAL(relations.size(), 1);
  BOOST_CHECK_EQUAL(relations[0], "student");

  // not supported
  sql = "select name - 1 from student;";
  BOOST_CHECK_THROW(sql_parse(sql, pc), ParserErrorInfo);
  pc.clear();
}

BOOST_AUTO_TEST_CASE(test_data_type) {
  // ---------------------------- data type: string / date
  std::string sql = "select name from student where ";
  std::string cond = "birthday > '2001-12-06 '";
  std::string semi = ";";
  auto sw_cond = [&sql, &semi](const std::string &cond) -> std::string { return sql + cond + semi; };

  ParserContext pc;
  int result = sql_parse(sw_cond(cond), pc);
  {
    BOOST_CHECK(result == 0);
    BOOST_CHECK_MESSAGE(check_stacks_empty(pc), "stacks should be empty");

    auto *query = pc.get_query<SelectQuery>();
    BOOST_CHECK(nullptr != query);
    auto &pred = query->get_conditions();
    BOOST_CHECK_EQUAL(static_cast<int>(pred.get_node_type()), static_cast<int>(PredicateType::CNT_LEAF));
    auto *leaf = dynamic_cast<PredicateLeaf *>(&pred);
    BOOST_CHECK(nullptr != leaf);
    BOOST_CHECK(leaf->get_op() == CompareOp::OP_GT);

    auto &lchild = leaf->get_lchild();
    auto &rchild = leaf->get_rchild();
    BOOST_CHECK(lchild.get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK(rchild.get_operand_type() == OperandType::COT_VAL);
    auto &lattr = dynamic_cast<OperandAttr *>(&lchild)->get_attr();
    BOOST_CHECK_EQUAL(lattr.get_attribute(), "birthday");
    auto &rval = dynamic_cast<const OperandVal *>(&rchild)->get_value();
    BOOST_CHECK(rval.get_value_type() == ValueType::VT_STRING);
    BOOST_CHECK_EQUAL(rval.get_as<String>(), "2001-12-06 ");
  }
  {
    // for
    cond = "'aa' = 'b == exit b'";
    result = sql_parse(sw_cond(cond), pc);
    BOOST_CHECK(0 == result);
    auto *query = pc.get_query<SelectQuery>();
    BOOST_CHECK(nullptr != query);
    auto &pred = query->get_conditions();
    BOOST_CHECK_EQUAL(static_cast<int>(pred.get_node_type()), static_cast<int>(PredicateType::CNT_LEAF));
    auto *leaf = dynamic_cast<PredicateLeaf *>(&pred);
    BOOST_CHECK(nullptr != leaf);
    BOOST_CHECK(leaf->get_op() == CompareOp::OP_EQ);

    auto &lchild = leaf->get_lchild();
    auto &rchild = leaf->get_rchild();
    BOOST_CHECK(lchild.get_operand_type() == OperandType::COT_VAL);
    BOOST_CHECK(rchild.get_operand_type() == OperandType::COT_VAL);
    auto &lval = dynamic_cast<const OperandVal *>(&lchild)->get_value();
    auto &rval = dynamic_cast<const OperandVal *>(&rchild)->get_value();

    BOOST_CHECK(lval.get_value_type() == ValueType::VT_STRING);
    BOOST_CHECK_EQUAL(lval.get_as<String>(), "aa");

    BOOST_CHECK(rval.get_value_type() == ValueType::VT_STRING);
    BOOST_CHECK_EQUAL(rval.get_as<String>(), "b == exit b");
  }
  // -------------------------- data type: float
  {
    cond = "3.0 <= credit";
    result = sql_parse(sw_cond(cond), pc);
    BOOST_CHECK(0 == result);
    auto *query = pc.get_query<SelectQuery>();
    auto &leaf = dynamic_cast<PredicateLeaf &>(query->get_conditions());
    BOOST_CHECK(leaf.get_op() == CompareOp::OP_LE);
    auto &lchild = leaf.get_lchild();
    BOOST_CHECK(lchild.get_operand_type() == OperandType::COT_VAL);
    auto &rchild = leaf.get_rchild();
    BOOST_CHECK(rchild.get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK_EQUAL(dynamic_cast<OperandAttr *>(&rchild)->get_attr().get_attribute(), "credit");
    auto &lval = dynamic_cast<const OperandVal *>(&lchild)->get_value();
    BOOST_CHECK(lval.get_value_type() == ValueType::VT_FLOAT);
    BOOST_CHECK_EQUAL(lval.get_as<Float>(), 3.0);
  }

  // -------------------------- data type
  {
    cond = "workingyear = 10";
    result = sql_parse(sw_cond(cond), pc);
    BOOST_CHECK(0 == result);
    auto *query = pc.get_query<SelectQuery>();
    auto &leaf = dynamic_cast<PredicateLeaf &>(query->get_conditions());
    BOOST_CHECK(leaf.get_op() == CompareOp::OP_EQ);
    auto &rchild = leaf.get_rchild();
    BOOST_CHECK(rchild.get_operand_type() == OperandType::COT_VAL);
    auto &lchild = leaf.get_lchild();
    BOOST_CHECK(lchild.get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK_EQUAL(dynamic_cast<OperandAttr *>(&lchild)->get_attr().get_attribute(), "workingyear");
    auto &rval = dynamic_cast<const OperandVal *>(&rchild)->get_value();
    BOOST_CHECK(rval.get_value_type() == ValueType::VT_INT);
    BOOST_CHECK_EQUAL(rval.get_as<Integer>(), 10);
  }
}

BOOST_AUTO_TEST_CASE(test_select_multi_attribute_and_relation) {
  auto sw_rel = [](const std::string &attrs, const std::string &rels) {
    return "select " + attrs + " from " + rels + ";";
  };
  ParserContext pc;
  {
    int result = sql_parse(sw_rel("a1, a2, a3", "t1, t2, t3, t4"), pc);
    BOOST_CHECK(result == 0);
    auto *query = pc.get_query<SelectQuery>();
    auto &relations = query->get_relations();
    auto &attributes = query->get_attributes();
    std::vector<std::string> s_attr{"a1", "a2", "a3"};
    for (int i = 0; i < 3; ++i) {
      BOOST_CHECK_EQUAL(attributes[i].get_attribute(), s_attr[i]);
    }
    std::vector<std::string> s_rel{"t1", "t2", "t3", "t4"};
    BOOST_CHECK(std::equal(relations.begin(), relations.end(), s_rel.begin()));
  }
  { BOOST_CHECK_THROW(sql_parse(sw_rel("", "t1, t2, t3, t4"), pc), ParserErrorInfo); }
  { BOOST_CHECK_THROW(sql_parse(sw_rel("a1, a2, a3", ""), pc), ParserErrorInfo); }
  {
    int result = sql_parse(sw_rel("*, a.b, c.s2", "t2"), pc);
    BOOST_CHECK(result == 0);
    auto *query = pc.get_query<SelectQuery>();
    auto &relations = query->get_relations();
    BOOST_CHECK(relations.size() == 1 && relations[0] == "t2");
    auto &attributes = query->get_attributes();
    BOOST_CHECK(attributes.size() == 3);
    BOOST_CHECK(attributes[0].get_relation() == "" && attributes[0].get_attribute() == "*");
    BOOST_CHECK(attributes[1].get_relation() == "a" && attributes[1].get_attribute() == "b");
    BOOST_CHECK(attributes[2].get_relation() == "c" && attributes[2].get_attribute() == "s2");
  }
}

BOOST_AUTO_TEST_CASE(test_select_predicate_conjunctions_precedence) {
// sql:`and or not` are in same precedence with c++
// works fine for both g++ and clang++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
  // not a and b
  BOOST_CHECK((not 0 and 0) == ((not 0) and 0));  // not (0 and 0) == 1
  // not a or c
  BOOST_CHECK((not 0 or 1) == ((not 0) or 1));  // not (0 or 1) == 0
  // a and b or c
  BOOST_CHECK((0 and 0 or 1) == ((0 and 0) or 1));  // 0 and (0 or 1) == 0;
  // a or b and c
  BOOST_CHECK((1 or 0 and 0) == (1 or (0 and 0)));  // (1 or 0) and 0 == 0;
#pragma GCC diagnostic pop

  std::string sql = "select name from student where ";
  std::string semi = ";";
  auto sw_cond = [&sql, &semi](const std::string &cond) -> std::string { return sql + cond + semi; };
  ParserContext pc;
  {
    // not a and b <> (not a) and b
    int result = sql_parse(sw_cond("not a = 1 and b < c"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_AND);
    auto *and_pred = dynamic_cast<PredicateAnd *>(&pred);
    BOOST_CHECK(and_pred != nullptr);
    auto &lchild = and_pred->get_lchild();
    BOOST_CHECK(lchild.get_node_type() == PredicateType::CNT_NOT);
    auto &not_pred = lchild.get_as<PredicateNot>();
    auto &l_leaf = not_pred.get_child().get_as<PredicateLeaf>();
    BOOST_CHECK(l_leaf.get_op() == CompareOp::OP_EQ);
    BOOST_CHECK(l_leaf.get_lchild().get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK(l_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "a");
    BOOST_CHECK_EQUAL(l_leaf.get_rchild().get_as<OperandVal>().get_value().get_as<Integer>(), 1);

    auto &r_child = and_pred->get_rchild();
    BOOST_CHECK(r_child.get_node_type() == PredicateType::CNT_LEAF);
    auto &r_leaf = r_child.get_as<PredicateLeaf>();
    BOOST_CHECK(r_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "b");
    BOOST_CHECK(r_leaf.get_rchild().get_as<OperandAttr>().get_attr().get_attribute() == "c");
  }

  {
    // not a or b <> (not a) or b
    int result = sql_parse(sw_cond("not a < 2.25 or 1 >= c"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_OR);
    auto *and_pred = dynamic_cast<PredicateOr *>(&pred);
    BOOST_CHECK(and_pred != nullptr);
    auto &lchild = and_pred->get_lchild();
    BOOST_CHECK(lchild.get_node_type() == PredicateType::CNT_NOT);
    auto &not_pred = lchild.get_as<PredicateNot>();
    auto &l_leaf = not_pred.get_child().get_as<PredicateLeaf>();
    BOOST_CHECK(l_leaf.get_op() == CompareOp::OP_LT);
    BOOST_CHECK(l_leaf.get_lchild().get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK(l_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "a");
    BOOST_CHECK_EQUAL(l_leaf.get_rchild().get_as<OperandVal>().get_value().get_as<Float>(), 2.25);

    auto &r_child = and_pred->get_rchild();
    BOOST_CHECK(r_child.get_node_type() == PredicateType::CNT_LEAF);
    auto &r_leaf = r_child.get_as<PredicateLeaf>();
    BOOST_CHECK(r_leaf.get_op() == CompareOp::OP_GE);
    BOOST_CHECK(r_leaf.get_lchild().get_as<OperandVal>().get_value().get_as<Integer>() == 1);
    BOOST_CHECK(r_leaf.get_rchild().get_as<OperandAttr>().get_attr().get_attribute() == "c");
  }

  {
    int result = sql_parse(sw_cond("not (a = 1 and b < c)"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_NOT);
    auto *not_pred = dynamic_cast<PredicateNot *>(&pred);
    BOOST_CHECK(not_pred != nullptr);
    auto *and_pred = dynamic_cast<PredicateAnd *>(&(not_pred->get_child()));
    auto &lchild = and_pred->get_lchild();
    BOOST_CHECK(lchild.get_node_type() == PredicateType::CNT_LEAF);
    auto &l_leaf = lchild.get_as<PredicateLeaf>();
    BOOST_CHECK(l_leaf.get_op() == CompareOp::OP_EQ);
    BOOST_CHECK(l_leaf.get_lchild().get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK(l_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "a");
    BOOST_CHECK_EQUAL(l_leaf.get_rchild().get_as<OperandVal>().get_value().get_as<Integer>(), 1);

    auto &r_child = and_pred->get_rchild();
    BOOST_CHECK(r_child.get_node_type() == PredicateType::CNT_LEAF);
    auto &r_leaf = r_child.get_as<PredicateLeaf>();
    BOOST_CHECK(r_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "b");
    BOOST_CHECK(r_leaf.get_rchild().get_as<OperandAttr>().get_attr().get_attribute() == "c");
  }

  {
    int result = sql_parse(sw_cond("not (a = 1 or b < c)"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_NOT);
    auto *not_pred = dynamic_cast<PredicateNot *>(&pred);
    BOOST_CHECK(not_pred != nullptr);
    auto *and_pred = dynamic_cast<PredicateOr *>(&(not_pred->get_child()));
    auto &lchild = and_pred->get_lchild();
    BOOST_CHECK(lchild.get_node_type() == PredicateType::CNT_LEAF);
    auto &l_leaf = lchild.get_as<PredicateLeaf>();
    BOOST_CHECK(l_leaf.get_op() == CompareOp::OP_EQ);
    BOOST_CHECK(l_leaf.get_lchild().get_operand_type() == OperandType::COT_ATTR);
    BOOST_CHECK(l_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute() == "a");
    BOOST_CHECK_EQUAL(l_leaf.get_rchild().get_as<OperandVal>().get_value().get_as<Integer>(), 1);

    auto &r_child = and_pred->get_rchild();
    BOOST_CHECK(r_child.get_node_type() == PredicateType::CNT_LEAF);
    auto &r_leaf = r_child.get_as<PredicateLeaf>();
    BOOST_CHECK(r_leaf.get_lchild().get_as<OperandAttr>().get_attr().get_relation().empty());
    BOOST_CHECK(r_leaf.get_rchild().get_as<OperandAttr>().get_attr().get_relation().empty());
  }

  {
    // a and b or c <> (a and b) or c
    int result = sql_parse(sw_cond("1 = 2 and 2 = a or c > 3"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_OR);
    BOOST_CHECK(pred.get_as<PredicateOr>().get_lchild().get_node_type() == PredicateType::CNT_AND);
    BOOST_CHECK(pred.get_as<PredicateOr>().get_rchild().get_node_type() == PredicateType::CNT_LEAF);
  }

  {
    // a or b and c <> a or (b and c)
    int result = sql_parse(sw_cond("1 = 2 or a = 1 and c > 1"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_OR);
    BOOST_CHECK(pred.get_as<PredicateOr>().get_lchild().get_node_type() == PredicateType::CNT_LEAF);
    BOOST_CHECK(pred.get_as<PredicateOr>().get_rchild().get_node_type() == PredicateType::CNT_AND);
  }

  {
    // (a or b) and c
    int result = sql_parse(sw_cond(" (1 = 2 or a =1) and c != '3'"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_AND);
    BOOST_CHECK(pred.get_as<PredicateAnd>().get_lchild().get_node_type() == PredicateType::CNT_OR);
    BOOST_CHECK(pred.get_as<PredicateAnd>().get_rchild().get_node_type() == PredicateType::CNT_LEAF);
  }

  {
    // a and (b or c)
    int result = sql_parse(sw_cond("c != '2' and (1 = 2 or a =1)"), pc);
    BOOST_CHECK(result == 0);
    auto &pred = (pc.get_query<SelectQuery>()->get_conditions());
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_AND);
    BOOST_CHECK(pred.get_as<PredicateAnd>().get_lchild().get_node_type() == PredicateType::CNT_LEAF);
    BOOST_CHECK(pred.get_as<PredicateAnd>().get_rchild().get_node_type() == PredicateType::CNT_OR);
  }
}