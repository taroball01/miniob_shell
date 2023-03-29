#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include "../util/utils.h"
#include "planner/plan_node.h"
#include "planner/planner.h"
#include "sql/predicate/operand.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_single_relation_select) {
  auto stmt = generate_stmt("select stuId, name from student where birthday = '2001-03-26';");
  BOOST_CHECK(nullptr != stmt);

  Planner planner;
  auto ptr = planner.plan_stmt(*stmt);
  BOOST_CHECK(ptr != nullptr);

  BOOST_CHECK(ptr->get_plan_node_type() == PlanNodeType::Projection);
  auto &projection = dynamic_cast<ProjectionPlanNode &>(*ptr);
  auto attr = projection.get_output_schema().get_schema_array();
  // sort by attribute
  sort(attr.begin(), attr.end(),
       [](const SchemaItem &lhs, const SchemaItem &rhs) { return lhs.attribute_ < rhs.attribute_; });
  BOOST_CHECK_EQUAL(attr[0].attribute_, "name");
  BOOST_CHECK_EQUAL(attr[1].relation_, "student");
  BOOST_CHECK_EQUAL(attr[1].attribute_, "stuId");

  BOOST_CHECK(projection.get_child().get_plan_node_type() == PlanNodeType::Filter);
  auto &filter = dynamic_cast<FilterPlanNode &>(projection.get_child());
  auto output_filter = filter.get_output_schema().get_schema_array();
  BOOST_CHECK_NE(attr.size(), output_filter.size());
  auto &pred = filter.get_condition();
  BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_LEAF);
  auto &leaf = dynamic_cast<const PredicateLeaf &>(pred);
  BOOST_CHECK(leaf.get_op() == CompareOp::OP_EQ);

  BOOST_CHECK(filter.get_child().get_plan_node_type() == PlanNodeType::MultiJoin);
  auto &mtj = dynamic_cast<MultiJoinPlanNode &>(filter.get_child());
  auto output_mtj = mtj.get_output_schema().get_schema_array();
  BOOST_CHECK(std::equal(output_filter.begin(), output_filter.end(), output_mtj.begin(), check_schema_item_equal));
  BOOST_CHECK_EQUAL(mtj.get_children().size(), 1);

  auto &rel = *(mtj.get_children().at(0));
  BOOST_CHECK(rel.get_plan_node_type() == PlanNodeType::TableScan);
  auto &ts = dynamic_cast<TableScanPlanNode &>(rel);
  BOOST_CHECK_EQUAL(ts.get_relation(), "student");
}

BOOST_AUTO_TEST_CASE(test_multi_relation_select) {
  auto stmt = generate_stmt(
      "select name, courseName, score from student, grade, course where student.stuId = grade.stuId and grade.courseId "
      "= course.courseId and score >= 90.0;");
  BOOST_CHECK(nullptr != stmt);

  Planner planner;
  auto ptr = planner.plan_stmt(*stmt);
  BOOST_CHECK(ptr != nullptr);

  BOOST_CHECK(ptr->get_plan_node_type() == PlanNodeType::Projection);
  auto &projection = dynamic_cast<ProjectionPlanNode &>(*ptr);
  auto attr = projection.get_output_schema().get_schema_array();
  sort(attr.begin(), attr.end(),
       [](const SchemaItem &lhs, const SchemaItem &rhs) { return lhs.attribute_ < rhs.attribute_; });
  BOOST_CHECK_EQUAL(attr.size(), 3);
  BOOST_CHECK_EQUAL(attr[0].attribute_, "courseName");
  BOOST_CHECK_EQUAL(attr[0].relation_, "course");
  BOOST_CHECK_EQUAL(attr[1].attribute_, "name");
  BOOST_CHECK_EQUAL(attr[1].relation_, "student");
  BOOST_CHECK_EQUAL(attr[2].attribute_, "score");
  BOOST_CHECK_EQUAL(attr[2].relation_, "grade");

  BOOST_CHECK(projection.get_child().get_plan_node_type() == PlanNodeType::Filter);
  auto &filter0 = dynamic_cast<FilterPlanNode &>(projection.get_child());
  auto output_filter0 = filter0.get_output_schema().get_schema_array();
  BOOST_CHECK_NE(attr.size(), output_filter0.size());
  {
    auto &pred = filter0.get_condition();
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_LEAF);
    auto &leaf = const_cast<PredicateLeaf &>(dynamic_cast<const PredicateLeaf &>(pred));
    BOOST_CHECK(leaf.get_op() == CompareOp::OP_GE);
    BOOST_CHECK_EQUAL(leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute(), "score");
    BOOST_CHECK(leaf.get_rchild().get_as<OperandVal>().get_value() == Float(90.0));
  }

  BOOST_CHECK(filter0.get_child().get_plan_node_type() == PlanNodeType::Filter);
  auto &filter1 = dynamic_cast<FilterPlanNode &>(filter0.get_child());
  auto output_filter1 = filter1.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(output_filter0.size(), output_filter1.size());
  {
    auto &pred = filter1.get_condition();
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_LEAF);
    auto &leaf = const_cast<PredicateLeaf &>(dynamic_cast<const PredicateLeaf &>(pred));
    BOOST_CHECK(leaf.get_op() == CompareOp::OP_EQ);
    BOOST_CHECK_EQUAL(leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute(), "courseId");
  }

  BOOST_CHECK(filter1.get_child().get_plan_node_type() == PlanNodeType::Filter);
  auto &filter2 = dynamic_cast<FilterPlanNode &>(filter1.get_child());
  auto output_filter2 = filter2.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(output_filter0.size(), output_filter2.size());
  {
    auto &pred = filter2.get_condition();
    BOOST_CHECK(pred.get_node_type() == PredicateType::CNT_LEAF);
    auto &leaf = const_cast<PredicateLeaf &>(dynamic_cast<const PredicateLeaf &>(pred));

    BOOST_CHECK(leaf.get_op() == CompareOp::OP_EQ);
    BOOST_CHECK_EQUAL(leaf.get_lchild().get_as<OperandAttr>().get_attr().get_attribute(), "stuId");
  }

  BOOST_CHECK(filter2.get_child().get_plan_node_type() == PlanNodeType::MultiJoin);
  auto &mtj = dynamic_cast<MultiJoinPlanNode &>(filter2.get_child());
  auto output_mtj = mtj.get_output_schema().get_schema_array();
  BOOST_CHECK_EQUAL(output_filter2.size(), output_mtj.size());
  BOOST_CHECK(std::equal(output_filter0.begin(), output_filter0.end(), output_mtj.begin(), check_schema_item_equal));
  auto &ts_arr = mtj.get_children();
  BOOST_CHECK_EQUAL(ts_arr.size(), 3);
  std::vector<std::string> relations;
  for (auto &ts : ts_arr) {
    auto &ts_r = dynamic_cast<TableScanPlanNode &>(*ts);
    relations.emplace_back(ts_r.get_relation());
  }
  sort(relations.begin(), relations.end());
  BOOST_CHECK_EQUAL(relations[2], "student");
  BOOST_CHECK_EQUAL(relations[1], "grade");
  BOOST_CHECK_EQUAL(relations[0], "course");
}