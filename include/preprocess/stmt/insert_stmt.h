#pragma once

#include "preprocess/stmt/statement.h"
#include "sql/query/insert.h"
#include "sql/query/query.h"
namespace query_process_engine {

class InsertStmt : public Statement {
private:
  InsertQuery r_query_;

public:
  explicit InsertStmt(InsertQuery&& rq) : r_query_(std::move(rq)) {}
  auto get_sql_type() const -> SqlType override { return SqlType::Insert; }
  auto get_query() -> InsertQuery& { return r_query_; }
};
} // namespace query_process_engine