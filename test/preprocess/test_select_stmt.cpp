#include <algorithm>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>
#include "../util/utils.h"
#include "preprocess/stmt/select_stmt.h"
#include "sql/predicate/compare_op.h"
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_construct_simple) {
  std::vector<Attribute> attrs = {
      {"", "courseId"},
      {"", "score"},
      {"grade", "stuId"},
  };

  SelectStmt select(db_schema, attrs, generate_predicate("a < b and c = 1 and 2 = 3"));
  BOOST_CHECK(select.get_sql_type() == SqlType::Select);
  auto &sch_arr = select.get_schema().get_schema_array();
  BOOST_CHECK_EQUAL(sch_arr.size(), select.get_schema().get_schema_array().size());
  BOOST_CHECK_EQUAL(attrs.size(), select.get_attributes().size());
  auto &cond = select.get_conditions();
  BOOST_CHECK_EQUAL(cond.size(), 3);

  auto &c0 = cond[0]->get_as<PredicateLeaf>();
  BOOST_CHECK(c0.get_op() == CompareOp::OP_LT);
  BOOST_CHECK_EQUAL(c0.get_lchild().get_as<OperandAttr>().get_attr().get_attribute(), "a");
  BOOST_CHECK_EQUAL(cond[1]->get_as<PredicateLeaf>().check_simple(), false);
  BOOST_CHECK_EQUAL(cond[2]->get_as<PredicateLeaf>().check_simple(), true);
}

BOOST_AUTO_TEST_CASE(test_split_and) {
  SelectStmt select(Schema{}, std::vector<Attribute>{}, nullptr);
  auto &cond = select.get_conditions();
  BOOST_CHECK_EQUAL(cond.size(), 0);
  select.set_conditions(nullptr);
  BOOST_CHECK_EQUAL(cond.size(), 0);

  // no and
  std::vector<std::string> no_and_arr = {" a< b", "not 1 !=2 ", "1 < d or c> 9", "a = 0 and 2 > 3 or d <= 3.4"};
  std::for_each(no_and_arr.begin(), no_and_arr.end(), [&select, &cond](const std::string &str) {
    select.set_conditions(generate_predicate(str));
    BOOST_CHECK_EQUAL(cond.size(), 1);
  });

  // a and b and c and d
  std::vector<std::string> linear_and_arr = {
      "a >8 and b < 3",
      "c != 2.3 and d < 2 and dd23  <= '23'",
      "ds_date < '2003-03-12' and bb = 0 and ccd < 2 and dfsds < 2.14",
  };
  int id = 2;
  std::for_each(linear_and_arr.begin(), linear_and_arr.end(), [&id, &select, &cond](const std::string &str) {
    select.set_conditions(generate_predicate(str));
    BOOST_CHECK_EQUAL(cond.size(), id);
    ++id;
  });

  std::string str = "(a_ < 34 or c != 0 )and not 2 > 4 and c > d";
  select.set_conditions(generate_predicate(str));
  BOOST_CHECK_EQUAL(cond.size(), 3);
  BOOST_CHECK(*cond[0] == *generate_predicate("a_ < 34 or c != 0"));
  BOOST_CHECK(*cond[1] == *generate_predicate(" not 2 > 4"));
  BOOST_CHECK(*cond[2] == *generate_predicate(" c > d "));
  // a and b or c
}
