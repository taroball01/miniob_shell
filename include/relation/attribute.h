#pragma once
#include <string>

namespace query_process_engine {
class Attribute {
 private:
  std::string relation_;
  std::string attribute_;

 public:
  explicit Attribute(const std::string &attr) : relation_(), attribute_(attr) {}
  Attribute(const std::string &attr, const std::string &rel) : relation_(rel), attribute_(attr) {}
  auto get_relation() const -> const std::string & { return relation_; }
  auto get_attribute() const -> const std::string & { return attribute_; }
  auto set_relation(const std::string &relation) -> void { relation_ = relation; }
  auto to_string() const -> std::string { return (relation_.empty() ? "" : (relation_ + ".")) + attribute_; }
  auto operator==(const Attribute &rhs) const -> bool {
    return relation_ == rhs.relation_ && attribute_ == rhs.attribute_;
  }
};
}  // namespace query_process_engine