#pragma once
#include "common/result_printer.h"

using namespace query_process_engine;

class NullResultPrinter : public ResultPrinter {
 public:
  NullResultPrinter() = default;
  auto output_error(const std::string &err) -> void override {}
  auto output_result(const std::vector<Tuple> &result) -> void override {}
  auto output_warn(const std::string &warn) -> void override {}
};
