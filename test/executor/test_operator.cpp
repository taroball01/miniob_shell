#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include "../util/utils.h"
#include "executor/operator.h"
#include "mock_storage_manager.h"
#include "planner/plan_node.h"
#include "relation/value/date.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_table_scan) {
  auto plan = generate_tablescan("student");
  TableScanOperator ts(mock_tsm, *plan);
  BOOST_CHECK_EQUAL(ts.open(), true);
  Tuple tp;
  bool result;
  result = ts.get_next(tp);
  BOOST_CHECK(result);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 3);
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'zeli'"));
  BOOST_CHECK(*tp.get_tuple_array().at(1) == Date("2001-06-10"));
  BOOST_CHECK(*tp.get_tuple_array().at(2) == Integer(3082));

  result = ts.get_next(tp);
  BOOST_CHECK(result);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 3);
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'taroball'"));
  BOOST_CHECK(*tp.get_tuple_array().at(1) == Date("2001-12-06"));
  BOOST_CHECK(*tp.get_tuple_array().at(2) == Integer(3084));

  result = ts.get_next(tp);
  BOOST_CHECK(result);
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'op'"));
  BOOST_CHECK(*tp.get_tuple_array().at(1) == Date("2001-02-28"));
  BOOST_CHECK(*tp.get_tuple_array().at(2) == Integer(3097));

  result = ts.get_next(tp);
  BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(test_filter_operator) {
  auto plan_ts = generate_tablescan("grade");
  auto ts = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts);
  auto plan_fl =
      std::make_unique<FilterPlanNode>(generate_tablescan("grade"), generate_predicate("grade.score <= 90.0"));
  FilterOperator filter(*plan_fl, std::move(ts));
  Tuple tp;
  bool result = filter.open();
  BOOST_CHECK_EQUAL(result, true);

  result = filter.get_next(tp);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 3);
  BOOST_CHECK(*tp.get_tuple_array().at(0) == Integer(23));
  BOOST_CHECK(*tp.get_tuple_array().at(1) == Integer(3084));
  BOOST_CHECK(*tp.get_tuple_array().at(2) == Float(71.0));
  result = filter.get_next(tp);
  BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(test_projection_operator) {
  auto plan_ts = generate_tablescan("teacher");
  auto ts = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts);
  std::vector<Attribute> attr = {
      Attribute{"faculty", "teacher"},
      Attribute{"teacherName", "teacher"},
  };
  auto plan_pr = std::make_unique<ProjectionPlanNode>(generate_tablescan("teacher"), attr);
  ProjectionOperator projection(*plan_pr, std::move(ts));
  Tuple tp;
  bool result = projection.open();
  BOOST_CHECK_EQUAL(result, true);

  result = projection.get_next(tp);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 2);
  BOOST_CHECK(*tp.get_tuple_array().at(1) == String("'b.liu'"));
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'cs'"));

  result = projection.get_next(tp);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 2);
  BOOST_CHECK(*tp.get_tuple_array().at(1) == String("'jg'"));
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'gis'"));

  result = projection.get_next(tp);
  BOOST_CHECK_EQUAL(result, true);
  BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 2);
  BOOST_CHECK(*tp.get_tuple_array().at(1) == String("'b.li'"));
  BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'cs'"));

  result = projection.get_next(tp);
  BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(test_nestloop_join_operator) {
  {
    auto plan_ts0 = generate_tablescan("student");
    auto ts0 = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts0);
    auto plan_ts1 = generate_tablescan("grade");
    auto ts1 = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts1);
    auto bnj = std::make_unique<BinaryJoinPlanNode>(generate_tablescan("student"), generate_tablescan("grade"));
    NestLoopJoinOperator nsj(*bnj, std::move(ts0), std::move(ts1));

    bool result = nsj.open();
    BOOST_CHECK_EQUAL(result, true);
    int number = 1;
    Tuple tp;
    result = nsj.get_next(tp);
    BOOST_CHECK_EQUAL(result, true);
    BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 6);
    BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'zeli'"));
    BOOST_CHECK(*tp.get_tuple_array().at(1) == Date("2001-06-10"));
    BOOST_CHECK(*tp.get_tuple_array().at(2) == Integer(3082));
    BOOST_CHECK(*tp.get_tuple_array().at(3) == Integer(33));
    BOOST_CHECK(*tp.get_tuple_array().at(4) == Integer(3082));
    BOOST_CHECK(*tp.get_tuple_array().at(5) == Float(95.0));

    while (nsj.get_next(tp)) {
      BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 6);
      ++number;
    }
    BOOST_CHECK_EQUAL(number, 9);
  }
  {
    auto plan_ts0 = generate_tablescan("student");
    auto ts0 = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts0);
    auto plan_ts1 = generate_tablescan("grade");
    auto ts1 = std::make_unique<TableScanOperator>(mock_tsm, *plan_ts1);
    auto bnj = std::make_unique<BinaryJoinPlanNode>(generate_tablescan("student"), generate_tablescan("grade"));
    auto plan_flt = std::make_unique<FilterPlanNode>(
        std::move(bnj), generate_predicate("student.stuId = grade.stuId and grade.score < 90.0"));
    auto bnj_op = std::make_unique<NestLoopJoinOperator>(dynamic_cast<BinaryJoinPlanNode &>(plan_flt->get_child()),
                                                         std::move(ts0), std::move(ts1));
    FilterOperator filter(*plan_flt, std::move(bnj_op));

    bool result = filter.open();
    BOOST_CHECK_EQUAL(result, true);
    Tuple tp;
    result = filter.get_next(tp);
    BOOST_CHECK_EQUAL(result, true);
    BOOST_CHECK_EQUAL(tp.get_tuple_array().size(), 6);
    BOOST_CHECK(*tp.get_tuple_array().at(0) == String("'taroball'"));
    BOOST_CHECK(*tp.get_tuple_array().at(1) == Date("2001-12-06"));
    BOOST_CHECK(*tp.get_tuple_array().at(2) == Integer(3084));
    BOOST_CHECK(*tp.get_tuple_array().at(3) == Integer(23));
    BOOST_CHECK(*tp.get_tuple_array().at(4) == Integer(3084));
    BOOST_CHECK(*tp.get_tuple_array().at(5) == Float(71.0));

    result = filter.get_next(tp);
    BOOST_CHECK_EQUAL(result, false);
  }
}