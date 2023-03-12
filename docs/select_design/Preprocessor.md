### Preprocessor

预处理器接收`ParserContext`中生成的`query`， 生成 扩充后的`Statement`

1. 扩充其中的属性
2. 进一步进行语法检查
3. 尝试简化条件子句



```cpp
class Preprocessor {
private:
    // need support from other part
    ITransactionalStorageManager& ts_manager_;
    // query process info collector
    QueryProcessLogger& logger_;
private:
    // 
    auto resolve_relations(const std::vector<std::string>& relations, std::unordered_map<std::string, Schema>& detailed_relations) -> bool;
    auto resolve_attributes(std::vector<Attribute>& attributes) -> bool;
    auto resolve_predicate_leaves(Predicate& conditions) -> bool;
    // add new predicate type to 
    auto pre_compute_simple_leaves();
    auto erase_not();
    auto split_and(std::vector<std::unique_ptr<Predicate>>& conditions);
private: 
    auto preprocess_select(SelectQuery& select) -> std::unique_ptr<SelectStmt>;
public:
    // all preprocess requests come to this method
    auto preprocess() -> std::unique_ptr<Stmt>;
};
```

```cpp
auto PreprocessSelect(SelectrQuery& query) -> std::unique_ptr<SelectStmt> {
    
    // check whether table exists, store schema
    resolve_relations(relations, stmt->get_relations());
    // check attributes for existence and inambuguity
    resolve_attributes(query.get_attributes())
    p_stmt->get_attributes()->swap(query.get_attributes);
    // check conditions for each leaf 
    reslove_predicate_leaves(query.conditions);
    // simplify condition
    auto pred = pre_compute_simple_leaves(query.conditions);
    pred = erase_not(pred);
    split_and(pred, p_stmt->get_conditions());
}
```

```cpp
class ITransactionalStorageManager {
public:
  	auto get_relation(std::string& rel) -> std::unique_ptr<Schema> = 0;  
};
```

```cpp
class SelectStmt : public Stmt{
private:
    std::unordered_map<std::string, Schema> relations_;
    std::vector<Attribute> attributes_;// 
    std::vector<std::unique_ptr<Predicate>> conditions; // simplified one 
};
```



```cpp
// add simple predicate type true / false
// shared_ptr or unique_ptr copy, using latter
auto pre_compute_simple_leaves(Predicate& condition) -> std::unique_ptr<Predicate> {
    if (condition.type = leaf) {
        if (check_simple()) {
            return make_unique<PredicateRaw>(true/false);
        } else {
            return make_unique<PredicateLeaf>(condition); // copy construct
        }
    } else {
        case not: auto child = pre_compute_simple_leaves(get_child); if (child.is_simple()) return child.reverse(); else return make_unique<PredicateNot>();
        
      	auto l = pre_compute_simple_leaves(lchild);
    	auto r = pre_compute_simple_leaves(rchild);
        case or: true return true; false return another; other make_unique<PredicateOr>();
        case and: true return another; false return false; other make_unique<PredicateAnd>();
    }
}
```



```cpp
auto erase_not(Predicate& conditions, bool current) -> std::unique_ptr<Predicate> {
	if (conditions.type == not) {
        return erase_not(condition.child, !current);
    } else if (and / or) {
        lchild = erase_not(lchild, current);
        rchild = erase_not(rchild, current);
        if (conditions.type == or) {
            return current ? make<or>() : make<and>();
        } else {
            return current ? make<and>() : make<or>();
        }
    } else {
        // leaf
        = -> !=;
        < -> >=;
        >= -> <;
    }
}
```

#### Test Cases

TBD