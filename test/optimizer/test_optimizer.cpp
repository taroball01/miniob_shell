#include <boost/test/unit_test.hpp>
#include "../util/utils.h"
#include "optimizer/optimizer.h"
#include "planner/plan_node.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_choose_join_sequence) {
  Optimizer optimizer;
  // raw
  {
    auto ptr = std::make_unique<MultiJoinPlanNode>();
    ptr->append_child(generate_tablescan("student"));
    auto nxt = optimizer.choose_join_sequence(std::move(ptr));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::TableScan);
  }

  {
    auto ptr = std::make_unique<MultiJoinPlanNode>();
    ptr->append_child(generate_tablescan("grade"));
    ptr->append_child(generate_tablescan("course"));
    auto nxt = optimizer.choose_join_sequence(std::move(ptr));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::BinaryJoin);
    auto &bj = dynamic_cast<BinaryJoinPlanNode &>(*nxt);
    BOOST_CHECK(bj.get_lchild().get_plan_node_type() == PlanNodeType::TableScan);
    BOOST_CHECK(bj.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
  }

  {
    auto ptr = std::make_unique<MultiJoinPlanNode>();
    ptr->append_child(generate_tablescan("enroll"));
    ptr->append_child(generate_tablescan("teacher"));
    ptr->append_child(generate_tablescan("teach"));
    auto nxt = optimizer.choose_join_sequence(std::move(ptr));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::BinaryJoin);
    auto &bj0 = dynamic_cast<BinaryJoinPlanNode &>(*nxt);
    BOOST_CHECK(bj0.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
    auto &ts1 = dynamic_cast<TableScanPlanNode &>(bj0.get_rchild());
    BOOST_CHECK_EQUAL(ts1.get_relation(), "teach");

    BOOST_CHECK(bj0.get_lchild().get_plan_node_type() == PlanNodeType::BinaryJoin);
    auto &bj2 = dynamic_cast<BinaryJoinPlanNode &>(bj0.get_lchild());
    BOOST_CHECK(bj2.get_lchild().get_plan_node_type() == PlanNodeType::TableScan);
    BOOST_CHECK(bj2.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
    auto &ts2 = dynamic_cast<TableScanPlanNode &>(bj2.get_lchild());
    auto &ts3 = dynamic_cast<TableScanPlanNode &>(bj2.get_rchild());
    BOOST_CHECK_EQUAL(ts2.get_relation(), "enroll");
    BOOST_CHECK_EQUAL(ts3.get_relation(), "teacher");
  }

  // multi- [proj, filter, bin, multi]
  {
    auto ptr = std::make_unique<MultiJoinPlanNode>();
    std::vector<Attribute> attrs = {Attribute{"name", "student"}};
    ptr->append_child(std::make_unique<ProjectionPlanNode>(generate_tablescan("student"), attrs));
    ptr->append_child(
        std::make_unique<FilterPlanNode>(generate_tablescan("enroll"), generate_predicate("stuId = 3084")));

    auto mtj = std::make_unique<MultiJoinPlanNode>();
    mtj->append_child(generate_tablescan("enroll"));
    mtj->append_child(generate_tablescan("teacher"));
    mtj->append_child(generate_tablescan("teach"));
    ptr->append_child(std::move(mtj));

    ptr->append_child(std::make_unique<BinaryJoinPlanNode>(generate_tablescan("course"), generate_tablescan("grade")));

    auto nxt = optimizer.choose_join_sequence(std::move(ptr));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::BinaryJoin);
    auto &bj0 = dynamic_cast<BinaryJoinPlanNode &>(*nxt);
    BOOST_CHECK(bj0.get_lchild().get_plan_node_type() == PlanNodeType::BinaryJoin);
    BOOST_CHECK(bj0.get_rchild().get_plan_node_type() == PlanNodeType::BinaryJoin);

    auto &bj_r = dynamic_cast<BinaryJoinPlanNode &>(bj0.get_rchild());
    BOOST_CHECK(bj_r.get_lchild().get_plan_node_type() == PlanNodeType::TableScan);
    BOOST_CHECK(bj_r.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
    auto &ts_rl = dynamic_cast<TableScanPlanNode &>(bj_r.get_lchild());
    auto &ts_rr = dynamic_cast<TableScanPlanNode &>(bj_r.get_rchild());
    BOOST_CHECK_EQUAL(ts_rl.get_relation(), "course");
    BOOST_CHECK_EQUAL(ts_rr.get_relation(), "grade");

    auto &bj_l = dynamic_cast<BinaryJoinPlanNode &>(bj0.get_lchild());
    BOOST_CHECK(bj_l.get_lchild().get_plan_node_type() == PlanNodeType::BinaryJoin);
    BOOST_CHECK(bj_l.get_rchild().get_plan_node_type() == PlanNodeType::BinaryJoin);

    auto &bj_ll = dynamic_cast<BinaryJoinPlanNode &>(bj_l.get_lchild());
    auto &bj_lr = dynamic_cast<BinaryJoinPlanNode &>(bj_l.get_rchild());
    BOOST_CHECK(bj_lr.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
    auto &ts_lrr = dynamic_cast<TableScanPlanNode &>(bj_lr.get_rchild());
    BOOST_CHECK_EQUAL(ts_lrr.get_relation(), "teach");

    BOOST_CHECK(bj_ll.get_lchild().get_plan_node_type() == PlanNodeType::Projection);
    BOOST_CHECK(bj_ll.get_rchild().get_plan_node_type() == PlanNodeType::Filter);
  }
  // [proj, filter, bin] - multi
  {
    auto mtj0 = std::make_unique<MultiJoinPlanNode>();
    mtj0->append_child(generate_tablescan("enroll"));
    std::vector<Attribute> attr = {Attribute{"stuId", "enroll"}};
    std::unique_ptr<PlanNode> projection = std::make_unique<ProjectionPlanNode>(std::move(mtj0), attr);
    projection = optimizer.choose_join_sequence(std::move(projection));
    BOOST_CHECK(projection->get_plan_node_type() == PlanNodeType::Projection);
    BOOST_CHECK(dynamic_cast<ProjectionPlanNode &>(*projection).get_child().get_plan_node_type() ==
                PlanNodeType::TableScan);

    auto mtj1 = std::make_unique<MultiJoinPlanNode>();
    mtj1->append_child(generate_tablescan("grade"));
    std::unique_ptr<PlanNode> filter =
        std::make_unique<FilterPlanNode>(std::move(mtj1), generate_predicate("score >= 60.0"));
    filter = optimizer.choose_join_sequence(std::move(filter));
    BOOST_CHECK(filter->get_plan_node_type() == PlanNodeType::Filter);
    BOOST_CHECK(dynamic_cast<FilterPlanNode &>(*filter).get_child().get_plan_node_type() == PlanNodeType::TableScan);

    auto mtj2 = std::make_unique<MultiJoinPlanNode>();
    mtj2->append_child(generate_tablescan("course"));
    auto mtj3 = std::make_unique<MultiJoinPlanNode>();
    mtj3->append_child(generate_tablescan("teacher"));
    std::unique_ptr<PlanNode> bj = std::make_unique<BinaryJoinPlanNode>(std::move(mtj2), std::move(mtj3));
    bj = optimizer.choose_join_sequence(std::move(bj));
    BOOST_CHECK(bj->get_plan_node_type() == PlanNodeType::BinaryJoin);
    auto &r_bj = dynamic_cast<BinaryJoinPlanNode &>(*bj);
    BOOST_CHECK(r_bj.get_lchild().get_plan_node_type() == PlanNodeType::TableScan);
    BOOST_CHECK(r_bj.get_rchild().get_plan_node_type() == PlanNodeType::TableScan);
  }
}

BOOST_AUTO_TEST_CASE(test_push_down_filter) {
  Optimizer optimizer;
  // filter - proj - [table]
  {
    std::vector<Attribute> attr = {Attribute{"courseId", "grade"}, Attribute{"score", "grade"}};
    auto projection = std::make_unique<ProjectionPlanNode>(generate_tablescan("grade"), attr);
    auto filter = std::make_unique<FilterPlanNode>(std::move(projection), generate_predicate("grade.courseId = 100"));
    auto nxt = optimizer.push_down_filter(std::move(filter));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::Projection);
    auto &pro = dynamic_cast<ProjectionPlanNode &>(*nxt);
    BOOST_CHECK(pro.get_child().get_plan_node_type() == PlanNodeType::Filter);
    auto &flt = dynamic_cast<FilterPlanNode &>(pro.get_child());
    BOOST_CHECK(flt.get_child().get_plan_node_type() == PlanNodeType::TableScan);
    auto ch_sch = flt.get_child().get_output_schema().get_schema_array();
    auto flt_sch = flt.get_output_schema().get_schema_array();
    BOOST_CHECK(std::equal(flt_sch.begin(), flt_sch.end(), ch_sch.begin(), check_schema_item_equal));
  }

  // filter - [binary]
  {
    // a | b | a and b
    auto bnj = std::make_unique<BinaryJoinPlanNode>(generate_tablescan("teach"), generate_tablescan("teacher"));
    auto filter_teacher =
        std::make_unique<FilterPlanNode>(std::move(bnj), generate_predicate("teacher.workingyears > 10"));
    auto filter_teach =
        std::make_unique<FilterPlanNode>(std::move(filter_teacher), generate_predicate("teach.courseId = 124"));
    auto filter_both = std::make_unique<FilterPlanNode>(std::move(filter_teach),
                                                        generate_predicate("teacher.teacherId = teach.teacherId"));

    auto nxt = optimizer.push_down_filter(std::move(filter_both));
    BOOST_CHECK(nxt->get_plan_node_type() == PlanNodeType::Filter);
    auto &filter_top = dynamic_cast<FilterPlanNode &>(*nxt);
    BOOST_CHECK(filter_top.get_condition() == *generate_predicate("teacher.teacherId = teach.teacherId"));
    BOOST_CHECK(filter_top.get_child().get_plan_node_type() == PlanNodeType::BinaryJoin);

    auto &r_bnj = dynamic_cast<BinaryJoinPlanNode &>(filter_top.get_child());
    BOOST_CHECK(r_bnj.get_lchild().get_plan_node_type() == PlanNodeType::Filter);
    BOOST_CHECK(r_bnj.get_rchild().get_plan_node_type() == PlanNodeType::Filter);
    auto &filter_left = dynamic_cast<FilterPlanNode &>(r_bnj.get_lchild());
    auto &filter_right = dynamic_cast<FilterPlanNode &>(r_bnj.get_rchild());
    BOOST_CHECK(filter_left.get_condition() == *generate_predicate("teach.courseId = 124"));
    BOOST_CHECK(filter_right.get_condition() == *generate_predicate("teacher.workingyears > 10"));
  }

  // multi filter - multi binary
  {
    // a | b | c | a and b | b and c | a and c | a and b and c
    // from student, grade, course where course.courseName = 'db' and grade.score = 96.0 and grade.stuId ==
    // student.stuId and grade.courseId = course.courseId;
    auto mtj = std::make_unique<MultiJoinPlanNode>();
    mtj->append_child(generate_tablescan("student"));
    mtj->append_child(generate_tablescan("grade"));
    mtj->append_child(generate_tablescan("course"));

    auto filter_course =
        std::make_unique<FilterPlanNode>(std::move(mtj), generate_predicate("course.courseName = 'db'"));
    auto filter_grade =
        std::make_unique<FilterPlanNode>(std::move(filter_course), generate_predicate("grade.score = 96.0"));
    auto filter_grade_student =
        std::make_unique<FilterPlanNode>(std::move(filter_grade), generate_predicate("grade.stuId = student.stuId"));
    auto filter_grade_course = std::make_unique<FilterPlanNode>(std::move(filter_grade_student),
                                                                generate_predicate("grade.courseId = course.courseId"));

    auto nxt = optimizer.choose_join_sequence(std::move(filter_grade_course));
    auto optimized = optimizer.push_down_filter(std::move(nxt));

    BOOST_CHECK(optimized->get_plan_node_type() == PlanNodeType::Filter);
    auto &filter_top = dynamic_cast<FilterPlanNode &>(*optimized);
    BOOST_CHECK(filter_top.get_condition() == *generate_predicate("grade.courseId = course.courseId"));
    BOOST_CHECK(filter_top.get_child().get_plan_node_type() == PlanNodeType::BinaryJoin);

    auto &bnj_top = dynamic_cast<BinaryJoinPlanNode &>(filter_top.get_child());
    BOOST_CHECK(bnj_top.get_lchild().get_plan_node_type() == PlanNodeType::Filter);
    BOOST_CHECK(bnj_top.get_rchild().get_plan_node_type() == PlanNodeType::Filter);
    auto &filter_l = dynamic_cast<FilterPlanNode &>(bnj_top.get_lchild());
    auto &filter_r = dynamic_cast<FilterPlanNode &>(bnj_top.get_rchild());

    BOOST_CHECK(filter_l.get_condition() == *generate_predicate("grade.stuId = student.stuId"));
    BOOST_CHECK(filter_r.get_condition() == *generate_predicate("course.courseName = 'db'"));

    BOOST_CHECK(filter_l.get_child().get_plan_node_type() == PlanNodeType::BinaryJoin);
    BOOST_CHECK(filter_r.get_child().get_plan_node_type() == PlanNodeType::TableScan);
    auto &ts_r = dynamic_cast<TableScanPlanNode &>(filter_r.get_child());
    BOOST_CHECK_EQUAL(ts_r.get_relation(), "course");

    auto &bnj_l = dynamic_cast<BinaryJoinPlanNode &>(filter_l.get_child());
    BOOST_CHECK(bnj_l.get_lchild().get_plan_node_type() == PlanNodeType::TableScan);
    BOOST_CHECK(bnj_l.get_rchild().get_plan_node_type() == PlanNodeType::Filter);

    auto &filter_lr = dynamic_cast<FilterPlanNode &>(bnj_l.get_rchild());
    BOOST_CHECK(filter_lr.get_condition() == *generate_predicate("grade.score = 96.0"));
  }
}