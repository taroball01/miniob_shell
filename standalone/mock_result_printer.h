#include <numeric>
#include <string>
#include <vector>
#include "common/parser_error/parser_error_info.h"
#include "common/parser_error/unknown_relation.h"
#include "common/result_printer.h"
#include "relation/schema.h"
#include "relation/value/value.h"

using namespace query_process_engine;

class MockResultPrinter : public ResultPrinter {
private:
  std::vector<int> column_width_;
  char row_span_ = '-';
  char col_span_ = '|';
  char space_ = ' ';
  char min_distance_ = 1;

private:
  auto calc_column_width(const std::vector<Tuple> &result) -> void {
    column_width_.resize(attributes_.size());
    int size = column_width_.size();
    for (int i=0; i < size; ++i) {
      column_width_[i] = attributes_[i].to_string().size();
    } 
    for (auto& tp : result) {
      auto& tp_arr = tp.get_tuple_array();
      for (int i=0;i<size;++i) {
        column_width_[i] = std::max<int>(column_width_[i], tp_arr.at(i)->to_string().size());
      }
    }
  }
  auto calc_column_width(const std::vector<std::string>& rels) -> void {
    column_width_.resize(attributes_.size());
    column_width_[0] = std::string("relation").size();
    for (auto& rel : rels) {
      column_width_[0] = std::max<int>(column_width_[0], rel.size());
    }
  }
  auto output_multi_char(char c, int size) const -> void {
    std::cout<<std::string(size, c);
  }
  auto output_line_span(int size) const -> void {
    std::cout<<std::string(size, row_span_)<<"\n";
  }
  auto output_spaces(int size) const -> void{
    std::cout<<std::string(size, space_);
  }
  auto line_length() const -> int {
    int sum = std::accumulate(column_width_.begin(), column_width_.end(), 0);
    return sum + column_width_.size() * (min_distance_ * 2 + 1) + 1;
  }
  auto output_header() const -> void {
    std::cout<<col_span_;
    int id = 0;
    for (auto& attr : attributes_) {
      int col_size = column_width_[id++] + min_distance_ * 2;
      auto str = attr.to_string();
      int val_size = str.size();
      int space_num = col_size - val_size - min_distance_;
      output_spaces(space_num);
      std::cout<<str<<space_<<col_span_;
    }
    std::cout<<"\n";
  }
  auto output_relation_row(const std::string& rel) const -> void {
    std::cout<<col_span_;
    int col_size = column_width_[0] + min_distance_ * 2;
    int val_size = rel.size();
    int space_num = col_size - val_size - min_distance_;
    output_spaces(space_num);
    std::cout<<rel<<space_<<col_span_<<"\n";
  }
  auto output_row(const Tuple& tp) const -> void {
    std::cout<<col_span_;
    int id = 0;
    for (auto& val : tp.get_tuple_array()) {
      int col_size = column_width_[id++] + min_distance_ * 2;
      auto str = val->to_string();
      int val_size = str.size();
      int space_num = col_size - val_size - min_distance_;
      output_spaces(space_num);
      std::cout<<str<<space_<<col_span_;
    }
    std::cout<<"\n";
  }
  auto output_parser_error_prefix() const -> void {
    output_multi_char('-', 6);
    output_multi_char(col_span_, 1);
    output_spaces(1);
  }
public:
  MockResultPrinter() = default;
  auto output_error(const std::string &err) -> void override { std::cerr << err << std::endl; }
  auto output_result(const std::vector<Tuple> &result) -> void override { 
    calc_column_width(result);

    output_header();
    output_line_span(line_length());
    for (auto& tp : result) {
      output_row(tp);
    } 
    std::cout<<std::endl;   
  }
  auto output_warn(const std::string &warn) -> void override { std::cerr << warn << std::endl; }
  auto output_parser_error(ParserErrorInfo& error) -> void override {
    std::cout<<"\n";
    output_parser_error_prefix();
    std::cout<<error.get_raw_string(sql_)<<"\n";
    
    output_parser_error_prefix();
    auto col = error.get_position().second;
    output_multi_char('_', col - 1);
    std::cout<<"^\n";

    std::cout<<error.get_details()<<"\n";
  }
  auto output_relations(const std::vector<std::string>& rels) -> void override {
    set_attributes({Attribute{"relation"}});
    calc_column_width(rels);

    output_header();
    output_line_span(line_length());
    for (auto& rel : rels) {
      output_relation_row(rel);
    }
    std::cout<<std::endl;
  }
  auto output_schema(const std::string& rel, const std::vector<SchemaItem>& sch) -> void override {
    if (sch.empty()) {
      UnknownRelation ur(rel);
      output_parser_error(ur);
      return ;
    }
    std::cout<<"create table "<<rel<<" (\n";
    int size = sch.size();
    for (int i=0;i<size;++i) {
      output_spaces(2);
      auto& attr = sch[i].attribute_;
      auto tp = sch[i].type_;
      std::cout<<attr<<space_<<valuetype_to_string(tp);
      if (i != size - 1) {
        std::cout<<",";
      }
      std::cout<<"\n";
    }
    std::cout<<");\n"<<std::endl;
  }
  auto output_message(const std::string& msg) -> void override {
    std::cout<<msg<<std::endl;
  }
};
