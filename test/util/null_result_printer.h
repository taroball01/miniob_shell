#pragma once
#include <vector>
#include "common/parser_error/parser_error_info.h"
#include "common/result_printer.h"

using namespace query_process_engine;

class NullResultPrinter : public ResultPrinter {
 public:
  NullResultPrinter() = default;
  auto output_error(const std::string &err) -> void override {}
  auto output_result(const std::vector<Tuple> &result) -> void override {}
  auto output_warn(const std::string &warn) -> void override {}
  auto output_parser_error(ParserErrorInfo &) -> void override {}
  auto output_relations(const std::vector<std::string> &) -> void override {}
  auto output_message(const std::string &) -> void override {}
};
