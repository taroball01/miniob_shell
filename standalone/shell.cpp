#include <iostream>
#include <stdexcept>
#include <vector>
#include "common/parser_error/parser_error_info.h"
#include "executor/executor.h"
#include "mock_storage_manager.h"
#include "mock_result_printer.h"

#include "optimizer/optimizer.h"
#include "planner/planner.h"
#include "preprocess/preprocessor.h"
#include "sql/parser.h"
#include "sql/query/query.h"

using namespace query_process_engine;
#define CASE(LABEL) \
  case LABEL:       \
    return #LABEL
const char *type2string(SqlType tp) {
  switch (tp) {
    CASE(SqlType::Exit);
    CASE(SqlType::Select);
    default:
      return "unknow-type";
  }
}
constexpr char DEL = 127;
constexpr char BS = '\b';

int main() {
  MockResultPrinter printer;
  while (true) {
    std::cout << ">\t" << std::flush;
    // read sql
    std::vector<char> sql;
    char nxt;
    bool semi = false;
    do {
      nxt = std::cin.get();
      if (nxt == DEL || nxt == BS) {
        if (!sql.empty()) sql.pop_back();
        continue;
      }
      semi |= (nxt == ';');
      sql.emplace_back(nxt);
    } while (!(semi && nxt == '\n'));
    sql.back() = '\0';
    printer.set_sql(sql.data());
    ParserContext context;

    try {
        // pass to parser, store message in context
      int rc = sql_parse(sql.data(), context);
      if (rc != 0 || nullptr == context.query_) {
        throw std::logic_error("Unknown parser failed reason.");
        continue;
      }

      SqlType type = context.query_->get_sql_type();
      switch (type) {
        case SqlType::Exit: {
          std::cout<<"Bye\n";
          return 0;
        }
        case SqlType::ShowTables: {
          printer.output_relations(mock_tsm.get_relations());
          continue;
        }
        case SqlType::DescTable: {
          auto& desc = dynamic_cast<DescTable&>(*context.query_);
          auto& rel = desc.get_relation_name();
          auto sch = mock_tsm.get_relation(rel);
          printer.output_schema(rel, sch);
          continue;
        }
        default: {
          break; // fall 
        }
      }
      Preprocessor preprocessor(mock_tsm, printer);
      auto stmt = preprocessor.preprocess(std::move(context.query_));

      Planner planner;
      auto plan = planner.plan_stmt(*stmt);

      Optimizer optimizer;
      plan = optimizer.optimize(std::move(plan));

      Executor executor(mock_tsm);
      auto physical_plan = executor.build_physical_operator(*plan);
      auto result = executor.execute(*physical_plan);
      printer.output_result(result);
    } catch(ParserErrorInfo& err) {
      printer.output_parser_error(err);
    } catch(const std::logic_error& e) {
      static std::string logic_prefix("Internal Engine Error: ");
      printer.output_error(logic_prefix+ e.what());
    } 

    std::cout << "\n";
  }
  return 0;
}