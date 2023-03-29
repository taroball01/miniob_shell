#include <boost/test/unit_test.hpp>
#include <memory>
#include <vector>
#include "../util/utils.h"
#include "relation/schema.h"
#include "relation/tuple.h"
#include "relation/value/date.h"
#include "relation/value/value.h"
#include "sql/predicate/predicate.h"

using namespace query_process_engine;

// test evaluate
BOOST_AUTO_TEST_CASE(test_predicate_evaluate) {
  std::vector<SchemaItem> schema_arr = {
      {"student", "stuId", ValueType::VT_INT},
      {"student", "name", ValueType::VT_STRING},
      {"student", "birthday", ValueType::VT_DATE},
      {"grade", "score", ValueType::VT_FLOAT},
  };
  Schema sch(schema_arr);
  Tuple tp;
  tp.append_back(std::make_shared<Integer>(20193081));
  tp.append_back(std::make_shared<String>("'ze.li'"));
  tp.append_back(std::make_shared<Date>("2001-06-10"));
  tp.append_back(std::make_shared<Float>(99.5));

  auto pred = generate_predicate("student.stuId <= 20193084");
  BOOST_CHECK(pred->evaluate(tp, sch));
  pred = generate_predicate("student.stuId > 20203000");
  BOOST_CHECK(!pred->evaluate(tp, sch));

  pred = generate_predicate("student.name != 'lize'");
  BOOST_CHECK(pred->evaluate(tp, sch));

  pred = generate_predicate("grade.score = 99.5");
  BOOST_CHECK(pred->evaluate(tp, sch));

  // manually change string to date
  pred = generate_predicate("student.birthday < '2001-12-06'");
  auto &r_date = dynamic_cast<PredicateLeaf &>(*pred).get_rchild();
  dynamic_cast<OperandVal &>(r_date).set_value(std::make_unique<Date>("2001-12-06"));
  BOOST_CHECK(pred->evaluate(tp, sch));

  pred = generate_predicate("student.stuId = 20193081 and grade.score > 95.0");
  BOOST_CHECK(pred->evaluate(tp, sch));

  pred = generate_predicate("student.stuId != 20193081 or grade.score < 95.0");
  BOOST_CHECK(!pred->evaluate(tp, sch));

  pred = generate_predicate("not 'lize' = student.name");
  BOOST_CHECK(pred->evaluate(tp, sch));

  pred = generate_predicate("not 'ze.li' = student.name");
  BOOST_CHECK(!pred->evaluate(tp, sch));
}

// test operator==
BOOST_AUTO_TEST_CASE(test_predicate_operator_equal) {
  BOOST_CHECK(*generate_predicate("student.stuId <= 20193084") == *generate_predicate("student.stuId <= 20193084"));
  auto pred =
      generate_predicate("not student.stuId != 20193081 or grade.score < 95.0 and course.courseName != 'database'");
  BOOST_CHECK(pred->get_node_type() == PredicateType::CNT_OR);
  auto &or_pred = dynamic_cast<PredicateOr &>(*pred);
  BOOST_CHECK(or_pred.get_lchild() == *generate_predicate(" not student.stuId != 20193081"));

  auto &and_pred = dynamic_cast<PredicateAnd &>(or_pred.get_rchild());
  BOOST_CHECK(and_pred.get_lchild() == *generate_predicate("grade.score < 95.0"));
  BOOST_CHECK(and_pred.get_rchild() == *generate_predicate(" course.courseName != 'database'"));
}

// test resolve for operand
BOOST_AUTO_TEST_CASE(test_predicate_resolve_operand) {
  std::vector<SchemaItem> sch_arr = {
      {"teacher", "faculty", ValueType::VT_STRING},
      {"course", "courseName", ValueType::VT_STRING},
      {"enroll", "courseId", ValueType::VT_INT},
  };
  Schema sch(sch_arr);

  Tuple tp;
  tp.append_back(std::make_shared<String>("'cs'"));
  tp.append_back(std::make_shared<String>("'database'"));
  tp.append_back(std::make_shared<Integer>(11223344));
  OperandAttr oprd(Attribute{"courseName", "course"});
  BOOST_CHECK(oprd.resolve_value(tp, sch) == String("'database'"));
  OperandAttr oprd2(Attribute{"courseId", "enroll"});
  BOOST_CHECK(oprd2.resolve_value(tp, sch) == Integer(11223344));
  OperandAttr oprd3(Attribute{"faculty", "teacher"});
  BOOST_CHECK(oprd3.resolve_value(tp, sch) == String("'cs'"));
}