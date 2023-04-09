#pragma once

#include "preprocess/stmt/statement.h"
#include "relation/schema.h"
#include "sql/query/delete.h"
#include "sql/query/query.h"
namespace query_process_engine {

class DeleteStmt : public Statement {
private:
  DeleteQuery r_query_;
  std::vector<SchemaItem> sch_;

public:
  explicit DeleteStmt(DeleteQuery&& rq, std::vector<SchemaItem> sch) : r_query_(std::move(rq)), sch_(sch) {}
  auto get_sql_type() const -> SqlType { return SqlType::Delete; }
  auto get_query() -> DeleteQuery& { return r_query_; }
  auto get_relation_schema() -> const std::vector<SchemaItem>& { return sch_; } 
};

} // namespace query_process_engine