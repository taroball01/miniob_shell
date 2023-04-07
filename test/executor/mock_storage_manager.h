#pragma once
#include <unordered_map>
#include <vector>
#include "common/storage_manager.h"

namespace query_process_engine {
class TupleId {
 public:
  std::string relation_;
  int offset_;
  TupleId(const std::string &rel, int off) : relation_(rel), offset_(off) {}
};
}  // namespace query_process_engine
using namespace query_process_engine;

class MockStorageManager : public ITranscationalStorageManager {
 private:
  Schema db_schema_;
  std::unordered_map<std::string, std::vector<Tuple>> data_;

 public:
  MockStorageManager(const Schema &, const std::unordered_map<std::string, std::vector<Tuple>> &);
  auto get_relation(const std::string &rel) -> std::vector<SchemaItem> override;
  auto get_start_id(const std::string &rel) -> std::shared_ptr<TupleId> override;
  auto get_next_id(const TupleId &id) -> std::shared_ptr<TupleId> override;
  auto get_tuple(const TupleId &id) -> Tuple override;
  auto get_relations() -> std::vector<std::string> override;
};

extern MockStorageManager mock_tsm;