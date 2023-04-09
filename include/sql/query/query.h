#pragma once
#include <string>
namespace query_process_engine {
enum class SqlType {
  Invalid,
  Exit,
  Select,
  ShowTables,
  DescTable,
  CreateTable,
  Insert,
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

class ShowTables : public Query {
public:
  auto get_sql_type() const -> SqlType override { return SqlType::ShowTables; }
};

class DescTable : public Query {
private:
  std::string rel_;
public:
  explicit DescTable(const std::string& rel) : rel_(rel) {}
  auto get_sql_type() const -> SqlType override { return SqlType::DescTable; }
  auto get_relation_name() const -> const std::string& { return rel_; }
};
}  // namespace query_process_engine