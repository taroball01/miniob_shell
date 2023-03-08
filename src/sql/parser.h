#pragma once
#include <string>
#include <vector>
#include <memory>

#include "value.h"
#include "condition.h"
#include "attribute.h"

enum class SqlType{
  EXIT,
  HELP,
  DROP_TABLE,
  SHOW_TABLES,
  DESC_TABLE,
  DROP_INDEX,
  CREATE_INDEX,
  INSERT,
  CREATE_TABLE,
  UPDATE,
  DELETE,
  DIRECT_SELECT,
};

class Query {
public:
  virtual SqlType get_sql_type() const = 0; 
	virtual ~Query(){};
};

class Exit : public Query {
public:
  SqlType get_sql_type() const override {
    return SqlType::EXIT;
  }
};

class Help : public Query {
public:
  SqlType get_sql_type() const override {
    return SqlType::HELP;
  }
};

class ShowTables : public Query {
public:
  SqlType get_sql_type() const override {
    return SqlType::SHOW_TABLES;
  }
};

class DropTable: public Query {
private:
  std::string relation_;

public:
  explicit DropTable(const char* relation) : relation_(relation) {}
  SqlType get_sql_type() const override {
    return SqlType::DROP_TABLE;
  }
};

class DescTable: public Query {
private:
  std::string relation_;

public:
  explicit DescTable(const char* relation) : relation_(relation) {}
  SqlType get_sql_type() const override {
    return SqlType::DESC_TABLE;
  }
};

class DropIndex: public Query {
private:
  std::string relation_;

public:
  explicit DropIndex(const char* relation) : relation_(relation) {}
  SqlType get_sql_type() const override {
    return SqlType::DROP_INDEX;
  }
};

class CreateIndex : public Query {
private:
  std::string index_;
  std::string relation_;
  std::string attribute_;

public:
  CreateIndex(const char* id, const char* rel, const char* attr) : index_(id), relation_(rel), attribute_(attr) {}
  SqlType get_sql_type() const override {
    return SqlType::CREATE_INDEX;
  }
};

struct SchemaItem {
  std::string attribute_;
  ValueType type;
};
class CreateTable : public Query {
private:
  std::string relation_;
  std::vector<SchemaItem> schema;
public:
  explicit CreateTable(const char* rel) : relation_(rel) {}
  SqlType get_sql_type() const override {
    return SqlType::CREATE_TABLE;
  }
  const std::vector<SchemaItem>& get_schema() const{
    return schema;
  }
  
  void add_schema(SchemaItem&& item) {
    schema.emplace_back(item);
  }
  void add_schema(const SchemaItem& item) {
    schema.emplace_back(item);
  }
  const std::string& get_relation() const {
    return relation_;
  }
};

class Insert : public Query {
private:
  std::string relation_;
  std::vector<std::string> attributes_;
  std::vector<std::unique_ptr<Value>> values_;
public:
  explicit Insert(const char* rel) : relation_(rel) {}
  
  SqlType get_sql_type() const override {
    return SqlType::INSERT;
  }

  void add_attribute(std::string&& str) {
    attributes_.emplace_back(str);
  }
  void add_attribute(const std::string& str) {
    attributes_.emplace_back(str);
  }

  void add_value(std::unique_ptr<Value>&& val) {
    values_.emplace_back(std::move(val));
  }

  const std::vector<std::string>& get_attributes() const {
    return attributes_;
  }
  const std::vector<std::unique_ptr<Value>>& get_values() const {
    return values_;
  }
};



struct UpdateSetItem {
  explicit UpdateSetItem(const char* attr) : attribute_(attr) {}
  std::string attribute_;
};
struct UpdateSetValue : public UpdateSetItem {
  UpdateSetValue(const char* attr, std::unique_ptr<Value>&& p_v) : UpdateSetItem(attr), r_val_(std::move(p_v)) {

  }
  std::unique_ptr<Value> r_val_;
};
struct UpdateSetAttr : public UpdateSetItem {
  UpdateSetAttr(const char* attr, const char* rhs) : UpdateSetItem(attr), r_attr_(rhs) {}
  std::string r_attr_;
};

class Update : public Query {
private:
  std::string relation_;
  std::vector<std::unique_ptr<UpdateSetItem>> set_list_;
  std::unique_ptr<Predicate> conditions_;
public:
  explicit Update(const char* rel) : relation_(rel) {}
  SqlType get_sql_type() const override {
    return SqlType::UPDATE;
  }

  void add_set_item(std::unique_ptr<UpdateSetItem>&& item) {
    set_list_.emplace_back(std::move(item));
  }
  const std::vector<std::unique_ptr<UpdateSetItem>>& get_set_list() const {
    return set_list_;
  }

  void set_conditions(std::unique_ptr<Predicate>&& cond) {
    conditions_ = std::move(cond);
  }
};

class Delete : public Query {
private:
  std::string relation_;
  std::unique_ptr<Predicate> conditions_;
public:
  Delete(const char* rel, std::unique_ptr<Predicate>&& cond) 
    : relation_(rel), conditions_(std::move(cond)) {}
  SqlType get_sql_type() const override {
    return SqlType::DELETE;
  }
};

class Select : public Query {
private:
  std::unique_ptr<Predicate> conditions_;
  std::vector<std::unique_ptr<Attribute>> sel_list_;
  // support at most 2 tables currently
  std::vector<std::string> from_list_; 
public:
  Select() = default;
  SqlType get_sql_type() const override {
    return SqlType::DIRECT_SELECT;
  }
  void add_sel_attribute(std::unique_ptr<Attribute>&& attr) {
    sel_list_.emplace_back(std::move(attr));
  }
  void add_from_relation(const char* rel) {
    from_list_.emplace_back(rel);
  }
  void set_conditions(std::unique_ptr<Predicate>&& cond) {
    conditions_ = std::move(cond);
  }
};

struct ParserContext {
  Query* query;
};
