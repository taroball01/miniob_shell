#pragma once
#include <memory>
#include <vector>
#include "relation/value/value.h"

namespace query_process_engine {
class TupleId;
class Tuple {
 private:
  std::vector<std::shared_ptr<Value>> tp_;
  // p_tp_id is reserved for further implementation
  std::shared_ptr<TupleId> p_tp_id_{nullptr};

 public:
  Tuple() = default;
  auto get_tuple_array() const -> const std::vector<std::shared_ptr<Value>> & { return tp_; }
  auto append_back(const std::shared_ptr<Value> &ptr) -> void { tp_.emplace_back(ptr); }

  auto get_id() const -> TupleId * { return p_tp_id_.get(); }
  auto set_id(std::shared_ptr<TupleId>& id) -> void { p_tp_id_ = id; }
  auto clear() -> void { tp_.clear(); }
};

template <typename... Args>
auto make_tuple(Args &&... args) -> Tuple {
  Tuple tp;
  (tp.append_back(std::make_shared<Args>(args)), ...);
  return tp;
}
}  // namespace query_process_engine