### Planner

`planner` 接受预处理好的`Statement`， 按其生成逻辑查询计划，对于select来说，这一部分较为简单。

```cpp
class Planner {
private:
    auto plan_select(SelectStmt& select) ->std::unique_ptr<PlanNode>;
public:
    auto plan_stmt(Statement& stmt) -> std::unique_ptr<PlanNode>;
};
```



定义好各逻辑节点

```cpp
enum class PlanNodeType {
  TableScan,
  MultiJoin,
  BinaryJoin,
  Filter,
  Projection,  
};

class TableScanPlanNode : public PlanNode {
private:
	std::string relation_;    
};
// 在初始的逻辑查询计划中, join所有表, 优化时再选择顺序
class MultiJoinPlanNode : public PlanNode {
private:
    std::vector<std::unique_ptr<PlanNode>> children_;
};

class ProjectionPlanNode : public PlanNode {
private:
    std::unique_ptr<PlanNode> child_;
    std::vector<Attribute> attributes_;
};

class BinaryJoinPlanNode : public PlanNode {
private:
    std::unique_ptr<PlanNode> lchild, rchild_;
    std::unique_ptr<Predicate> condition;
};

class FilterPlanNode : public PlanNode{
private:
    std::unique_ptr<PlanNode> child_;
    std::unique_ptr<Predicate> condition;
}

```



```cpp
auto plan_select(SelectStmt& select) ->std::unique_ptr<PlanNode> {
   	auto ptr = make_unique<MultiJoinPlanNode>();
    for (select.relations_.keys()) {
        ptr->add_relation(make_unique<TableScanPlanNode>(cur));
    }
    ptr = make_unique<FilterPlanNode>(ptr, select.conditions);
    ptr = make_unique<ProjectionPlanNode>(ptr, select.conditions);
	return ptr;
}
```

