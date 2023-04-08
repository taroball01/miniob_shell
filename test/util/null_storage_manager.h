#pragma once
#include <vector>
#include "common/storage_manager.h"
using namespace query_process_engine;

class NullStorageManager : public ITranscationalStorageManager {
 private:
  Schema schema_;

 public:
  NullStorageManager() = default;
  auto get_relation(const std::string &rel) -> std::vector<SchemaItem> override {
    return schema_.get_relation_schema(rel);
  }
  auto get_relations() -> std::vector<std::string> override { return schema_.get_relations(); }
  auto get_start_id(const std::string &) -> std::shared_ptr<TupleId> override { return nullptr; }
  auto get_next_id(const TupleId &) -> std::shared_ptr<TupleId> override { return nullptr; }
  auto get_tuple(const TupleId &) -> Tuple override { return Tuple{}; }
  auto set_schema(Schema sch) -> void { schema_ = sch; }
  auto create_table(const std::vector<SchemaItem> &) -> bool override { return false; }
};