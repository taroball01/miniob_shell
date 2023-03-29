#include "utils.h"
#include "null_result_printer.h"
#include "null_storage_manager.h"
#include "planner/plan_node.h"
#include "preprocess/preprocessor.h"
#include "relation/schema.h"
#include "sql/parser.h"
#include "sql/query/select.h"

using namespace query_process_engine;
// db_schema_item_array
std::vector<SchemaItem> db_schema_item_array = {
    {"student", "name", ValueType::VT_STRING},    {"student", "birthday", ValueType::VT_DATE},
    {"student", "stuId", ValueType::VT_INT},

    {"enroll", "stuId", ValueType::VT_INT},       {"enroll", "courseId", ValueType::VT_INT},

    {"course", "courseId", ValueType::VT_INT},    {"course", "courseName", ValueType::VT_STRING},
    {"course", "credit", ValueType::VT_FLOAT},

    {"grade", "courseId", ValueType::VT_INT},     {"grade", "stuId", ValueType::VT_INT},
    {"grade", "score", ValueType::VT_FLOAT},

    {"teacher", "teacherId", ValueType::VT_INT},  {"teacher", "teacherName", ValueType::VT_STRING},
    {"teacher", "faculty", ValueType::VT_STRING}, {"teacher", "workingyears", ValueType::VT_INT},

    {"teach", "courseId", ValueType::VT_INT},     {"teach", "teacherId", ValueType::VT_INT},
};
// schema
Schema db_schema(db_schema_item_array);

auto generate_predicate(const std::string &cond) -> std::unique_ptr<Predicate> {
  static const std::string prefix("select * from a where ");
  ParserContext context;
  sql_parse(prefix + cond + ";", context);
  auto ptr = dynamic_cast<SelectQuery *>(context.query_.get());
  assert(ptr != nullptr);
  return ptr->transfer_conditions();
}

// avoid to misuse unique_ptr<>.operator==
auto check_same_predicate(const Predicate &l, const Predicate &r) -> bool { return l == r; }

auto generate_query(const std::string &sql) -> std::unique_ptr<Query> {
  ParserContext context;
  int result = sql_parse(sql, context);
  if (result != 0) {
    return nullptr;
  }
  return std::move(context.query_);
}

auto generate_stmt(const std::string &sql) -> std::unique_ptr<Statement> {
  auto query = generate_query(sql);
  NullStorageManager nsm;
  NullResultPrinter rp;
  Preprocessor preprocessor(nsm, rp);
  nsm.set_schema(db_schema);

  return preprocessor.preprocess(std::move(query));
}

auto check_schema_item_equal(const SchemaItem &lhs, const SchemaItem &rhs) -> bool {
  return lhs.attribute_ == rhs.attribute_ && lhs.relation_ == rhs.relation_ && lhs.type_ == rhs.type_;
};

auto generate_tablescan(const std::string &str) -> std::unique_ptr<TableScanPlanNode> {
  return std::make_unique<TableScanPlanNode>(str, db_schema.get_relation_schema(str));
}