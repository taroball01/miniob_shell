#pragma once
#include <string>

class Attribute {
private:
  std::string relation_;
  std::string attribute_;
public:
  explicit Attribute(const std::string& attr)
    :relation_(), attribute_(attr) {}
  Attribute(const std::string& attr, const std::string& rel)
    :relation_(rel), attribute_(attr) {}
  auto get_relation() const -> const std::string& {
    return relation_;
  }
  auto get_attribute() const -> const std::string& {
    return attribute_;
  }
};