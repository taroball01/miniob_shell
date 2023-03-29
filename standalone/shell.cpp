#include <iostream>
#include <stdexcept>
#include <vector>
#include "executor/executor.h"
#include "mock_storage_manager.h"
#include "mock_result_printer.h"

#include "optimizer/optimizer.h"
#include "planner/planner.h"
#include "preprocess/preprocessor.h"
#include "sql/parser.h"

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

int main() {
  while (true) {
    std::cout << ">\t" << std::flush;
    // read sql
    std::vector<char> sql;
    char nxt;
    bool semi = false;
    do {
      nxt = std::cin.get();
      semi |= (nxt == ';');
      sql.emplace_back(nxt);
    } while (!(semi && nxt == '\n'));
    sql.back() = '\0';
    ParserContext context;
    // pass to parser, store message in context
    int result = sql_parse(sql.data(), context);
    if (result != 0 || nullptr == context.query_) {
      continue;
    }

    SqlType type = context.query_->get_sql_type();
    std::cout << type2string(type)<<"\n";

    if (type == SqlType::Exit) {
      std::cout << "Bye\n";
      return 0;
    }
    MockResultPrinter printer;
    try {
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
    } catch(const std::runtime_error& e) {
      printer.output_error(e.what());
    } catch(const std::logic_error& e) {
      static std::string logic_prefix("Internal Engine Error: ");
      printer.output_error(logic_prefix+ e.what());
    } 

    std::cout << "\n";
  }
  return 0;
}