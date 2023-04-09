#pragma once
#include <string>
#include <vector>
#include "relation/schema.h"
#include "relation/tuple.h"

namespace query_process_engine {

class TupleId;
class ITranscationalStorageManager {
 public:
  virtual ~ITranscationalStorageManager() = default;
  virtual auto get_relation(const std::string &rel) -> std::vector<SchemaItem> = 0;
  virtual auto get_start_id(const std::string &) -> std::shared_ptr<TupleId> = 0;
  virtual auto get_next_id(const TupleId &id) -> std::shared_ptr<TupleId> = 0;
  virtual auto get_tuple(const TupleId &id) -> Tuple = 0;
  virtual auto get_relations() -> std::vector<std::string> = 0;
  virtual auto create_table(const std::vector<SchemaItem> &sch) -> bool = 0;
  virtual auto insert_tuple(const std::string &, std::vector<std::unique_ptr<Value>> &) -> bool = 0;
  virtual auto delete_tuple(const TupleId& id) -> bool = 0;
};
}  // namespace query_process_engine