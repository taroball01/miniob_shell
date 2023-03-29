#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <stdexcept>
#include "../util/null_result_printer.h"
#include "../util/null_storage_manager.h"
#include "../util/utils.h"
#include "preprocess/preprocessor.h"
#include "sql/predicate/predicate.h"
using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_resolve_relations) {
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  nsm.set_schema(db_schema);

  Schema output;
  std::vector<std::string> tables = {"student"};
  bool result = preprocessor.resolve_relations(tables, output);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(output.get_relations().size(), 1);
  // not in
  tables.emplace_back("db_course");
  BOOST_CHECK_THROW(preprocessor.resolve_relations(tables, output), std::runtime_error);

  tables.pop_back();
  tables = {"student", "teach", "enroll", "course", "grade", "teacher"};
  output.clear();
  result = preprocessor.resolve_relations(tables, output);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(output.get_relations().size(), tables.size());
  sort(tables.begin(), tables.end());
  auto out_tables = output.get_relations();
  sort(out_tables.begin(), out_tables.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), out_tables.begin(), out_tables.end());
}

BOOST_AUTO_TEST_CASE(test_resolve_attributes) {
  auto &sch = db_schema;
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  // right single attributes
  std::vector<Attribute> attrs = {Attribute{"name"}, Attribute{"score"}, Attribute{"credit"},
                                  Attribute{"workingyears"}};
  bool result = preprocessor.resolve_attributes(attrs, sch);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(attrs[0].get_relation(), "student");
  BOOST_CHECK_EQUAL(attrs[1].get_relation(), "grade");
  BOOST_CHECK_EQUAL(attrs[2].get_relation(), "course");
  BOOST_CHECK_EQUAL(attrs[3].get_relation(), "teacher");

  // ambiguous attributes
  attrs = {
      Attribute{"stuId", ""},
  };
  BOOST_CHECK_THROW(preprocessor.resolve_attributes(attrs, sch), std::runtime_error);
  attrs[0].set_relation("student");
  result = preprocessor.resolve_attributes(attrs, sch);
  BOOST_CHECK_EQUAL(result, true);

  attrs.emplace_back(Attribute{"courseId"});
  BOOST_CHECK_THROW(preprocessor.resolve_attributes(attrs, sch), std::runtime_error);

  attrs[1].set_relation("teach");
  result = preprocessor.resolve_attributes(attrs, sch);
  BOOST_CHECK_EQUAL(result, true);

  attrs.emplace_back(Attribute{"teacherId"});
  BOOST_CHECK_THROW(preprocessor.resolve_attributes(attrs, sch), std::runtime_error);

  attrs[2].set_relation("teacher");
  BOOST_CHECK_EQUAL(result, true);
  // attributes not occurred
  attrs = {Attribute{"birthday"}, Attribute{"strange_"}};
  BOOST_CHECK_THROW(preprocessor.resolve_attributes(attrs, sch), std::runtime_error);

  attrs = {Attribute{"birthday", "teacher"}};
  BOOST_CHECK_THROW(preprocessor.resolve_attributes(attrs, sch), std::runtime_error);

  // *
  attrs = {
      Attribute{"birthday", "number"},
      Attribute{"*"},
  };
  result = preprocessor.resolve_attributes(attrs, sch);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(attrs.size(), sch.get_schema_array().size());
}

BOOST_AUTO_TEST_CASE(test_resolve_predicate_leaves) {
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  auto &sch = db_schema;
  // check val - val
  std::vector<std::pair<std::string, bool>> test_cases = {{"1.0 > 1", false},
                                                          {" 3.0 < '1' ", false},
                                                          {"3.3 = 0.9", true},
                                                          {" 3 < 1 ", true},
                                                          {"'2tt3' = '23213 == dfadsa'", true},
                                                          {"23 != 'dsffad'", false}};

  for (auto &[prd_str, is_right] : test_cases) {
    auto ptr = generate_predicate(prd_str);
    if (is_right) {
      bool result = preprocessor.resolve_predicate_leaves(*ptr, sch);
      BOOST_CHECK_EQUAL(result, is_right);
    } else {
      BOOST_CHECK_THROW(preprocessor.resolve_predicate_leaves(*ptr, sch), std::runtime_error);
    }
  }

  // check attr - val
  test_cases = {
      {"grade.score > 65", false},    {"course.credit < '3.0'", false},  {"score <= 90.0", true},  // unique attribute
      {"student.stuId = 9090", true}, {"courseName = 'db'", true},       {"workingyears < '10'", false},

      {"birthday > '----'", false},   {"birthday = '2023-03-26'", true},
  };
  for (auto &[prd_str, is_right] : test_cases) {
    auto ptr = generate_predicate(prd_str);
    if (is_right) {
      bool result = preprocessor.resolve_predicate_leaves(*ptr, sch);
      BOOST_CHECK_EQUAL(result, is_right);
    } else {
      BOOST_CHECK_THROW(preprocessor.resolve_predicate_leaves(*ptr, sch), std::runtime_error);
    }
  }

  // check val - attr
  test_cases = {
      {" 65 > grade.score", false},    {"'3.0' <= course.credit ", false}, {"90.0 > score ", true},  // unique attribute
      {"9090 <= student.stuId", true}, {"'db' = courseName ", true},       {"'10' > workingyears", false},
      {"'----' < birthday", false},    {"'2023-03-26' = birthday", true},
  };
  for (auto &[prd_str, is_right] : test_cases) {
    auto ptr = generate_predicate(prd_str);
    if (is_right) {
      bool result = preprocessor.resolve_predicate_leaves(*ptr, sch);
      BOOST_CHECK_EQUAL(result, is_right);
    } else {
      BOOST_CHECK_THROW(preprocessor.resolve_predicate_leaves(*ptr, sch), std::runtime_error);
    }
  }
  // check attr - attr
  test_cases = {
      {" student.stuId < grade.score", false},
      {"name <= course.credit ", false},
      {"credit > score ", true},  // unique attribute
      {"teacher.teacherId <= student.stuId", true},
      {"teacherName = courseName ", true},
      {"name > workingyears", false},
  };
  for (auto &[prd_str, is_right] : test_cases) {
    auto ptr = generate_predicate(prd_str);
    if (is_right) {
      bool result = preprocessor.resolve_predicate_leaves(*ptr, sch);
      BOOST_CHECK_EQUAL(result, is_right);
    } else {
      BOOST_CHECK_THROW(preprocessor.resolve_predicate_leaves(*ptr, sch), std::runtime_error);
    }
  }
}

BOOST_AUTO_TEST_CASE(test_pre_compute_simple_leaves) {
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  // check val - val
  std::vector<std::pair<std::string, bool>> test_cases = {
      {"2.3 > 4.5", false},    {"3.3 = 3.3", true},       {"3.3 != 3.2", true},
      {" 1 > -3", true},       {" 4 < -23", false},       {"3452 = 3452", true},
      {"'abc' < 'abd'", true}, {"'@34$' != '###'", true}, {"'321123aa' = 'aa123321'", false},
  };
  // single leaves
  for (auto &[prd_str, val] : test_cases) {
    auto ptr = generate_predicate(prd_str);
    BOOST_CHECK(nullptr != ptr);
    ptr = preprocessor.pre_compute_simple_leaves(std::move(ptr));
    BOOST_CHECK(nullptr != ptr);
    auto &raw = dynamic_cast<PredicateRaw &>(*ptr);
    BOOST_CHECK_EQUAL(raw.get_value(), val);
  }
  // and support
  std::vector<std::pair<std::string, std::string>> test_cases_and = {
      {"2.3 > 4.5 and 3.3 = 3.3", "2.3 > 4.5"},                            // 0 and 1
      {"3.3 = 3.3 and 3.3 != 3.2", "3.3 = 3.3"},                           // 1 and 1
      {"3.3 != 3.3 and 2.3 > 4.5", "3.3 != 3.3"},                          // 0 and 0
      {" 1 > -3 and 'abv' = 'abd'", "'321123aa' = 'op'"},                  // 1 and 0
      {" 4 < -23 and stuId = 1", "4 < -23"},                               // 0 and predicate
      {"3452 = 3452 and id > 3 ", "id > 3"},                               // 1 and predicate
      {"not a = b and 'abc' > 'abd'", "'abc' > 'abd' "},                   // predicate and 0
      {"(not a= b or c <= d) and '@34$' != '###'", "not a= b or c <= d"},  // predicate and 1
  };
  for (auto &[prd_str, simplify] : test_cases_and) {
    auto l = generate_predicate(prd_str);
    auto r = generate_predicate(simplify);
    l = preprocessor.pre_compute_simple_leaves(std::move(l));
    r = preprocessor.pre_compute_simple_leaves(std::move(r));
    BOOST_CHECK(check_same_predicate(*l, *r));
  }
  // or support
  std::vector<std::pair<std::string, std::string>> test_cases_or = {
      {"2.3 > 4.5 or 3.3 = 3.3", "2.3 < 4.5"},                        // 0 or 1
      {"3.3 = 3.3 or 3.3 != 3.2", "3.3 = 3.3"},                       // 1 or 1
      {"3.3 != 3.3 or 2.3 > 4.5", "3.3 != 3.3"},                      // 0 or 0
      {" 1 > -3 or 'abv' = 'abd'", "'321123aa' != 'op'"},             // 1 or 0
      {" 4 < -23 or stuId = 1", "stuId = 1"},                         // 0 or predicate
      {"3452 = 3452 or id > 3 ", "12 <= 12"},                         // 1 or predicate
      {"not a = b or 'abc' > 'abd'", "not a = b "},                   // predicate or 0
      {"(not a= b or c <= d) or '@34$' != '###'", "'@34$' != '###"},  // predicate or 1
  };
  for (auto &[prd_str, simplify] : test_cases_and) {
    auto l = generate_predicate(prd_str);
    auto r = generate_predicate(simplify);
    l = preprocessor.pre_compute_simple_leaves(std::move(l));
    r = preprocessor.pre_compute_simple_leaves(std::move(r));
    BOOST_CHECK(check_same_predicate(*l, *r));
  }
  // not support
  std::vector<std::pair<std::string, std::string>> test_cases_not = {
      {"not 2.3 > 4.5", "2.3 <= 4.5"},  // not 0
      {"not 3.3 = 3.3", "3.3 != 3.3"},  // not 1
      {"not a = b", "not a=  b"},       // not simple

      {" not(4 < -23 and stuId = 1)", "4 >= -23"},                                   // not( 0 and predicate)
      {"not (3452 = 3452 and id > 3) ", "not id > 3"},                               // not(1 and predicate)
      {"not(not a = b and 'abc' > 'abd')", "not ('abc' > 'abd') "},                  // predicate and 0
      {"not((not a= b or c <= d) and '@34$' != '###')", "not(not a= b or c <= d)"},  // predicate and 1

      {"not( 4 < -23 or stuId = 1)", "not(stuId = 1)"},                        // 0 or predicate
      {"not(3452 = 3452 or id > 3) ", " not 12 <= 12"},                        // 1 or predicate
      {"not(not a = b or 'abc' > 'abd')", "not not a = b "},                   // predicate or 0
      {"not((not a= b or c <= d) or '@34$' != '###')", "not '@34$' != '###"},  // predicate or 1
  };
  for (auto &[prd_str, simplify] : test_cases_and) {
    auto l = generate_predicate(prd_str);
    auto r = generate_predicate(simplify);
    l = preprocessor.pre_compute_simple_leaves(std::move(l));
    r = preprocessor.pre_compute_simple_leaves(std::move(r));
    BOOST_CHECK(check_same_predicate(*l, *r));
  }
}

BOOST_AUTO_TEST_CASE(test_preprocess_select) {
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  nsm.set_schema(db_schema);
  // generate query
  {
    auto query = generate_query(
        "select name, grade.stuId from student, grade where score > 90.0 and courseId = 123 and 1 <= 2;");
    BOOST_CHECK(nullptr != query);
    auto stmt = preprocessor.preprocess(std::move(query));
    BOOST_CHECK(nullptr != stmt);
    BOOST_CHECK(stmt->get_sql_type() == SqlType::Select);
    auto &select = dynamic_cast<SelectStmt &>(*stmt);

    auto &sch = select.get_schema();
    BOOST_CHECK_EQUAL(sch.get_relations().size(), 2);
    auto &cond = select.get_conditions();
    BOOST_CHECK_EQUAL(cond.size(), 2);

    auto &attr = select.get_attributes();
    BOOST_CHECK_EQUAL(attr.size(), 2);
    BOOST_CHECK_EQUAL(attr[0].get_attribute(), "name");
    BOOST_CHECK_EQUAL(attr[0].get_relation(), "student");
  }
  // teacher teach *
  {
    auto query = generate_query("select * from teach, teacher where workingyears > 5 or 1 < 2;");
    auto stmt = preprocessor.preprocess(std::move(query));
    BOOST_CHECK(nullptr != stmt);
    BOOST_CHECK(stmt->get_sql_type() == SqlType::Select);
    auto &select = dynamic_cast<SelectStmt &>(*stmt);

    auto &sch = select.get_schema();
    BOOST_CHECK_EQUAL(sch.get_relations().size(), 2);
    auto &cond = select.get_conditions();
    BOOST_CHECK_EQUAL(cond.size(), 1);
    BOOST_CHECK(cond[0]->get_node_type() == PredicateType::CNT_RAW);

    auto &attr = select.get_attributes();
    BOOST_CHECK_EQUAL(attr.size(), 6);
  }
}