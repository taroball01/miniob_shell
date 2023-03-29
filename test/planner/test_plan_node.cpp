#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include "../util/utils.h"
#include "planner/plan_node.h"
#include "relation/schema.h"

using namespace query_process_engine;

// For plannode, now it only needs to test output schema
BOOST_AUTO_TEST_CASE(test_simple_output_schema) {
  Schema &sch = db_schema;
  auto check_schema_item_equal = [](const SchemaItem &lhs, const SchemaItem &rhs) -> bool {
    return lhs.attribute_ == rhs.attribute_ && lhs.relation_ == rhs.relation_ && lhs.type_ == rhs.type_;
  };
  // table scan
  const auto &stu_tb = sch.get_relation_schema("student");
  TableScanPlanNode ts("student", stu_tb);
  auto &ts_sch = ts.get_output_schema().get_schema_array();
  BOOST_CHECK(std::equal(ts_sch.begin(), ts_sch.end(), stu_tb.begin(), check_schema_item_equal));

  // projection
  std::vector<Attribute> attr = {Attribute{"name", "student"}, Attribute{"birthday", "student"}};
  ProjectionPlanNode projection(std::make_unique<TableScanPlanNode>(ts), attr);
  auto &pro_sch = projection.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(pro_sch.size(), 2);
  BOOST_CHECK_EQUAL(pro_sch[0].attribute_, "name");
  BOOST_CHECK_EQUAL(pro_sch[1].attribute_, "birthday");
  BOOST_CHECK_EQUAL(pro_sch[0].relation_, "student");
  BOOST_CHECK_EQUAL(pro_sch[1].relation_, "student");

  // filter
  const auto &grade_tb = sch.get_relation_schema("grade");
  TableScanPlanNode ts_grade("grade", grade_tb);
  FilterPlanNode filter(std::make_unique<TableScanPlanNode>(ts_grade), generate_predicate("1 < 2"));
  auto &fil_sch = filter.get_output_schema().get_schema_array();
  BOOST_CHECK(std::equal(fil_sch.begin(), fil_sch.end(), grade_tb.begin(), check_schema_item_equal));

  // BinaryJoin
  BinaryJoinPlanNode bnj(std::make_unique<TableScanPlanNode>(ts),
                         std::make_unique<TableScanPlanNode>("grade", grade_tb));
  auto &bnj_sch = bnj.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(bnj_sch.size(), grade_tb.size() + stu_tb.size());
  BOOST_CHECK(std::equal(stu_tb.begin(), stu_tb.end(), bnj_sch.begin(), check_schema_item_equal));
  BOOST_CHECK(std::equal(bnj_sch.begin() + stu_tb.size(), bnj_sch.end(), grade_tb.begin(), check_schema_item_equal));

  // multijoin
  MultiJoinPlanNode mtj;
  auto teach_tb = sch.get_relation_schema("teach");
  mtj.append_child(std::make_unique<TableScanPlanNode>("teach", teach_tb));
  mtj.append_child(std::make_unique<TableScanPlanNode>("grade", grade_tb));
  mtj.append_child(std::make_unique<TableScanPlanNode>("grade", stu_tb));
  auto mtj_sch = mtj.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(mtj_sch.size(), grade_tb.size() + stu_tb.size() + teach_tb.size());
  BOOST_CHECK(
      std::equal(mtj_sch.begin(), mtj_sch.begin() + teach_tb.size(), teach_tb.begin(), check_schema_item_equal));
  BOOST_CHECK(std::equal(grade_tb.begin(), grade_tb.end(), mtj_sch.begin() + teach_tb.size(), check_schema_item_equal));
  BOOST_CHECK(std::equal(stu_tb.begin(), stu_tb.end(), mtj_sch.begin() + teach_tb.size() + grade_tb.size(),
                         check_schema_item_equal));
}
