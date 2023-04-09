#include "mock_storage_manager.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>
#include "relation/tuple.h"
#include "relation/value/date.h"
#include "relation/value/value.h"

MockStorageManager::MockStorageManager(const std::unordered_map<std::string, std::vector<SchemaItem>> &sch,
                                       const std::unordered_map<std::string, std::vector<Tuple>> &data)
    : db_schema_(sch), data_(data) {}

auto MockStorageManager::get_relation(const std::string &rel) -> std::vector<SchemaItem> {
  return db_schema_.count(rel) ? db_schema_.at(rel) : std::vector<SchemaItem>{};
}

auto MockStorageManager::get_start_id(const std::string &rel) -> std::shared_ptr<TupleId> {
  if (data_.find(rel) == data_.end() || data_.at(rel).empty()) {
    return nullptr;
  }
  return std::make_shared<TupleId>(rel, 0);
}

auto MockStorageManager::get_next_id(const TupleId &id) -> std::shared_ptr<TupleId> {
  auto [rel, off] = id;
  ++off;
  int size = data_.at(rel).size();
  if (data_.find(rel) == data_.end() || off >= size) {
    return nullptr;
  }
  return std::make_shared<TupleId>(rel, off);
}

auto MockStorageManager::get_tuple(const TupleId &id) -> Tuple {
  auto &[rel, off] = id;
  int size = data_.at(rel).size();
  if (data_.find(rel) == data_.end() || off >= size) {
    return {};
  }
  return data_.at(rel).at(off);
}

auto MockStorageManager::get_relations() -> std::vector<std::string> {
  std::vector<std::string> relations;
  for (const auto& [key, _] : db_schema_) {
    relations.emplace_back(key);
  }
  std::sort(relations.begin(), relations.end());
  return relations;
}

auto MockStorageManager::create_table(const std::vector<SchemaItem>& sch) -> bool {
  if (sch.empty()) {
    return false;
  }
  auto& tb = sch[0].relation_;
  if (db_schema_.find(tb) != db_schema_.end()) {
    return false;
  }
  db_schema_.emplace(tb, sch);
  return true;
}

auto MockStorageManager::insert_tuple(const std::string &rel, std::vector<std::unique_ptr<Value>> &arr) -> bool {
  if (db_schema_.find(rel) == db_schema_.end()) {
    return false;
  }
  Tuple tp;
  for (auto& val : arr) {
    tp.append_back(std::move(val));
  }
  data_[rel].emplace_back(tp);
  return true;
}
static std::vector<Tuple> student_data = {
    make_tuple(String("'zeli'"), Date("2001-06-10"), Integer(3082)),
    make_tuple(String("'taroball'"), Date("2001-12-06"), Integer(3084)),
    make_tuple(String("'op'"), Date("2001-02-28"), Integer(3097)),
};
static std::vector<Tuple> enroll_data = {
    make_tuple(Integer(3082), Integer(33)),
    make_tuple(Integer(3084), Integer(23)),
    make_tuple(Integer(3097), Integer(31)),
};

static std::vector<Tuple> course_data = {
    make_tuple(Integer(31), String("'db'"), Float(3.0)),
    make_tuple(Integer(33), String("'kyxl1'"), Float(1.0)),
    make_tuple(Integer(23), String("'kyxl2'"), Float(1.0)),
};
static std::vector<Tuple> grade_data = {
    make_tuple(Integer(33), Integer(3082), Float(95.0)),
    make_tuple(Integer(23), Integer(3084), Float(71.0)),
    make_tuple(Integer(31), Integer(3097), Float(95.5)),
};
static std::vector<Tuple> teacher_data = {
    make_tuple(Integer(1001), String("'b.liu'"), String("'cs'"), Integer(10)),
    make_tuple(Integer(2001), String("'jg'"), String("'gis'"), Integer(12)),
    make_tuple(Integer(1003), String("'b.li'"), String("'cs'"), Integer(8)),
};
static std::vector<Tuple> teach_data = {
    make_tuple(Integer(31), Integer(1001)),
    make_tuple(Integer(33), Integer(2001)),
    make_tuple(Integer(23), Integer(1003)),
};
static std::unordered_map<std::string, std::vector<Tuple>> data = {
    {"student", student_data}, {"enroll", enroll_data},   {"course", course_data},
    {"grade", grade_data},     {"teacher", teacher_data}, {"teach", teach_data},
};


static std::vector<SchemaItem> student_schema = {
  {"student", "name", ValueType::VT_STRING},
  {"student", "birthday", ValueType::VT_DATE},
  {"student", "stuId", ValueType::VT_INT},
};
static std::vector<SchemaItem> enroll_schema = {
  {"enroll", "stuId", ValueType::VT_INT},       
  {"enroll", "courseId", ValueType::VT_INT},
};
static std::vector<SchemaItem> course_schema = {
  {"course", "courseId", ValueType::VT_INT},    
  {"course", "courseName", ValueType::VT_STRING},
  {"course", "credit", ValueType::VT_FLOAT},
};
static std::vector<SchemaItem> grade_schema = {
  {"grade", "courseId", ValueType::VT_INT},     
  {"grade", "stuId", ValueType::VT_INT},
  {"grade", "score", ValueType::VT_FLOAT},
};
static std::vector<SchemaItem> teacher_schema = {
  {"teacher", "teacherId", ValueType::VT_INT},  
  {"teacher", "teacherName", ValueType::VT_STRING},
  {"teacher", "faculty", ValueType::VT_STRING}, 
  {"teacher", "workingyears", ValueType::VT_INT},
};
static std::vector<SchemaItem> teach_schema = {
  {"teach", "courseId", ValueType::VT_INT},     
  {"teach", "teacherId", ValueType::VT_INT},
};

static std::unordered_map<std::string, std::vector<SchemaItem>> db_schema = {
  {"student", student_schema}, {"enroll", enroll_schema},   {"course", course_schema},
    {"grade", grade_schema},     {"teacher", teacher_schema}, {"teach", teach_schema},
};

MockStorageManager mock_tsm(db_schema, data);