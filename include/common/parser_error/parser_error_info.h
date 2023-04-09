#pragma once 
#include <exception>
#include <string>

namespace query_process_engine {
enum class ParserErrorType {
  Unknown_Character,
  Unexpected_Token, 
  Mismatched_Type,
  Unknown_Attribute,
  Unknown_Relation,
  Ambiguous_Attribute,
  Mismatched_Size,
};

class ParserErrorInfo : public std::exception {
private: 
  static constexpr std::pair<int, int> default_pr_ {1, 1};
public:
  static auto match_raw(const std::string& sql, int line) -> std::string {
    std::string ret;
    int id = 0;
    int size = sql.size();
    for (int i=1;i<line;++i) {
      while (id < size && sql[id] !='\n') {
        ++id;
      }
      ++id;
    }
    while (sql[id] != '\n' && sql[id] != '\0') {
      ret += sql[id] == '\t' ? ' ' : sql[id];
      ++id;
    }
    return ret;
  }
  virtual auto get_position() -> std::pair<int,int> {
    return default_pr_;
  }
  virtual auto get_raw_string(const std::string& sql) const -> std::string = 0;
  virtual auto get_error_type() const -> ParserErrorType = 0;
  virtual auto get_details() const -> std::string = 0;
  virtual ~ParserErrorInfo() = default;
};

} // namespace query_process_engine