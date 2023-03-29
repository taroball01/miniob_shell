#pragma once
namespace query_process_engine {
enum class SqlType {
  Invalid,
  Exit,
  Select,
};

class Query {
 public:
  virtual ~Query() = default;
  virtual auto get_sql_type() const -> SqlType = 0;
};

class ExitCmd : public Query {
 public:
  auto get_sql_type() const -> SqlType override { return SqlType::Exit; }
};
}  // namespace query_process_engine