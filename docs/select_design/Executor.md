### Executor

```cpp
class physical_operator {
public:
    auto open() -> bool = 0;
    auto get_next() -> bool = 0;
    auto close() -> void = 0;
};
```

