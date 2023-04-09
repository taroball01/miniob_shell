#pragma once

#include "sql/query/query.h"

namespace query_process_engine {
class Statement {
 public:
  virtual ~Statement() = default;
  virtual auto get_sql_type() const -> SqlType = 0;
};
}  // namespace query_process_engine