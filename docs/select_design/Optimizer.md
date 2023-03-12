### Optimizer

Optimizer 内部自实现一些优化规则，同时可交由用户注册自定义的优化规则。

```cpp
class Optimizer {
private:
    std::vector<optimze_func_t> custom_optimzers_;
    // Find MultiJoin, convert it into binary join; currently we only support construt a left-deep tree.
    auto choose_join_sequence(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode>;
    // Push down
    auto push_down_filter(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode>;
public:
    using optimize_func_t = std::function<auto(std::unique_ptr<PlanNode>)->std::unique_ptr<PlanNode>>;
    auto optimize(std::unique_ptr<PlanNode> ptr) -> std::unique_ptr<PlanNode>;
    auto register_optimzer(optimize_func_t ) -> void;
};
```

```cpp
auto optimize(std::unique_ptr<PlanNode> ptr) -> std::unique_ptr<PlanNode> {
    ptr = choose_join_sequence(ptr);
    ptr = push_down_filter(ptr);
    for_each(custom_optimizers_.begin(), custom_optimizers_.end(), [&ptr](optimze_func_t& opt) {
        ptr = opt(ptr);
    });
}
```



```cpp
auto push_down_filter(std::unique_ptr<PlanNode>) -> std::unique_ptr<PlanNode> {
    // first tranverse the tree and collect relation information for each BinaryJoin node
    // [] -> {a: left, b: right}
    // then for each filter, try to push as low as possible, or just store in binary join node.
}
```

