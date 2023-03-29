#pragma once

#include <iostream>
#include <vector>
#include "relation/attribute.h"
#include "relation/tuple.h"

namespace query_process_engine {
// This is a temporary class
class ResultPrinter {
 protected:
  std::vector<Attribute> attributes_;

 public:
  ResultPrinter() = default;
  explicit ResultPrinter(const std::vector<Attribute> &attr) : attributes_(attr) {}
  virtual ~ResultPrinter() = default;
  virtual auto output_error(const std::string &err) -> void { std::cerr << err << std::endl; }
  virtual auto output_result(const std::vector<Tuple> &result) -> void { throw; }
  virtual auto output_warn(const std::string &warn) -> void { std::cerr << warn << std::endl; }
  auto set_attributes(const std::vector<Attribute> &attributes) -> void { attributes_ = attributes; }
};

}  // namespace query_process_engine
