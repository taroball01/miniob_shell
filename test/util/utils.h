#pragma once
#include <memory>
#include <string>
#include "planner/plan_node.h"
#include "preprocess/stmt/statement.h"
#include "relation/schema.h"
#include "relation/value/value.h"
#include "sql/predicate/predicate.h"
#include "sql/query/query.h"
// db
extern std::vector<query_process_engine::SchemaItem> db_schema_item_array;
// schema
extern query_process_engine::Schema db_schema;

// put several util functions for test
auto generate_predicate(const std::string &cond) -> std::unique_ptr<query_process_engine::Predicate>;

auto check_same_predicate(const query_process_engine::Predicate &l, const query_process_engine::Predicate &r) -> bool;

auto generate_query(const std::string &sql) -> std::unique_ptr<query_process_engine::Query>;

auto generate_stmt(const std::string &sql) -> std::unique_ptr<query_process_engine::Statement>;

auto check_schema_item_equal(const query_process_engine::SchemaItem &lhs, const query_process_engine::SchemaItem &rhs)
    -> bool;

auto generate_tablescan(const std::string &str) -> std::unique_ptr<query_process_engine::TableScanPlanNode>;