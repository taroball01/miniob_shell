#pragma once

#include <iostream>
#include <vector>
#include "common/parser_error/parser_error_info.h"
#include "relation/attribute.h"
#include "relation/schema.h"
#include "relation/tuple.h"

namespace query_process_engine {
// This is a temporary class
class ResultPrinter {
 protected:
  std::vector<Attribute> attributes_;
  std::string sql_;

 public:
  ResultPrinter() = default;
  explicit ResultPrinter(const std::vector<Attribute> &attr) : attributes_(attr) {}
  virtual ~ResultPrinter() = default;
  virtual auto output_error(const std::string &err) -> void { std::cerr << err << std::endl; }
  virtual auto output_result(const std::vector<Tuple> &result) -> void { throw; }
  // for `show tables;`
  virtual auto output_relations(const std::vector<std::string> &) -> void {}
  // for `desc table;`
  virtual auto output_schema(const std::string &, const std::vector<SchemaItem> &) -> void {}
  virtual auto output_warn(const std::string &warn) -> void { std::cerr << warn << std::endl; }
  auto set_attributes(const std::vector<Attribute> &attributes) -> void { attributes_ = attributes; }
  auto set_sql(const std::string &sql) -> void { sql_ = sql; }
  virtual auto output_message(const std::string &msg) -> void = 0;
  virtual auto output_parser_error(ParserErrorInfo &error) -> void = 0;
};

}  // namespace query_process_engine
