#include "planner/plan_node.h"
#include <vector>
#include "relation/value/value.h"
namespace query_process_engine {

const std::vector<SchemaItem> InsertPlanNode::output_schema_array {std::vector<SchemaItem>{SchemaItem{"", "affected_rows", ValueType::VT_INT}}};

const std::vector<SchemaItem> DeletePlanNode::output_schema_array {
  std::vector<SchemaItem>{
    SchemaItem{"", "delete succeed", ValueType::VT_INT},
    SchemaItem{"", "delete failed", ValueType::VT_INT},
  }
};

} // namespace query_process_engine
