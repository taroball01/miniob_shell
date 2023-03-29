#pragma once
#include <memory>

namespace query_process_engine {
// dynamic cast for unique_ptr
template <typename To, typename From>
auto dynamic_unique_cast(std::unique_ptr<From> &&p) -> std::unique_ptr<To> {
  if (To *cast = dynamic_cast<To *>(p.get())) {
    std::unique_ptr<To> result(cast);
    p.release();
    return result;
  }
  return std::unique_ptr<To>(nullptr);  // or throw std::bad_cast() if you prefer
}

}  // namespace query_process_engine