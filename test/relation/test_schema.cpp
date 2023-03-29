#include <boost/test/unit_test.hpp>
#include "../util/utils.h"
#include "relation/schema.h"
#include "relation/value/value.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_schema_operation) {
  Schema sch;
  BOOST_CHECK(sch.get_schema_array().size() == 0);
  sch.append_schema({"student", "name", ValueType::VT_STRING});
  BOOST_CHECK(sch.is_attribute_exists("student", "name"));
  BOOST_CHECK_EQUAL(sch.get_relation("name"), "student");

  sch.append_schema({"teacher", "name", ValueType::VT_STRING});
  BOOST_CHECK_EQUAL(sch.get_attribute_id("teacher", "name"), 1);

  BOOST_CHECK_EQUAL(sch.count_relation("name"), 2);
  BOOST_CHECK_EQUAL(sch.get_relation("name"), "");

  sch.append_schema({"grade", "score", ValueType::VT_FLOAT});
  auto rels = sch.get_relations();
  std::vector<std::string> expected{"student", "teacher", "grade"};
  std::sort(rels.begin(), rels.end());
  std::sort(expected.begin(), expected.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(rels.begin(), rels.end(), expected.begin(), expected.end());
  BOOST_CHECK_EQUAL(sch.get_relation("score"), "grade");
}

BOOST_AUTO_TEST_CASE(test_schema_from_vector) {
  Schema sch(db_schema_item_array);
  BOOST_CHECK_EQUAL(sch.get_schema_array().size(), db_schema_item_array.size());
  BOOST_CHECK(sch.is_attribute_exists("teach", "courseId"));
  BOOST_CHECK(!sch.is_attribute_exists("teach", "workingyears"));
  BOOST_CHECK(!sch.is_attribute_exists("", "teacherId"));

  BOOST_CHECK_EQUAL(sch.get_attribute_id("teacher", "faculty"), 13);
  BOOST_CHECK(sch.get_attribute_schema("teacher", "faculty").type_ == ValueType::VT_STRING);

  BOOST_CHECK_EQUAL(sch.count_relation("courseId"), 4);
  BOOST_CHECK_EQUAL(sch.count_relation(""), 0);
  BOOST_CHECK_EQUAL(sch.get_relation("name"), "student");
  BOOST_CHECK_EQUAL(sch.get_relation("birthday"), "student");

  auto rel = sch.get_relations();
  std::vector<std::string> expected{"student", "enroll", "course", "grade", "teacher", "teach"};
  std::sort(rel.begin(), rel.end());
  std::sort(expected.begin(), expected.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(rel.begin(), rel.end(), expected.begin(), expected.end());
}