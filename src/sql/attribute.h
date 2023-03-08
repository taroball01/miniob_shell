#pragma once
#include <string>

class Attribute {
private:
  std::string relation_;
  // TODO: I'm not sure whether to use a new type to show wildcards, currently simply do this 
  std::string attribute_;
public:
  explicit Attribute(const char* attr)
    :relation_(), attribute_(attr) {}
  Attribute(const char* attr, const char* rel)
    :relation_(rel), attribute_(attr) {}
  bool is_wildcards() const { return attribute_ == "*"; }
};