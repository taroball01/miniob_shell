#include <numeric>
#include <string>
#include "common/parser_error/parser_error_info.h"
#include "common/result_printer.h"

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
      column_width_[i] = std::max<int>(column_width_[i], attributes_[i].to_string().size());
    } 
    for (auto& tp : result) {
      auto& tp_arr = tp.get_tuple_array();
      for (int i=0;i<size;++i) {
        column_width_[i] = std::max<int>(column_width_[i], tp_arr.at(i)->to_string().size());
      }
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
    for (auto& attr : attributes_) {
      std::cout<<space_<<attr.to_string()<<space_<<col_span_;
    }
    std::cout<<"\n";
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
};
