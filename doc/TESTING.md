# JConfigParser 测试文档

## 测试概述

JConfigParser 使用 doctest 作为单元测试框架，提供了全面的测试用例，确保代码质量和功能正确性。

## 快速开始

### 运行所有测试

```bash
cd build
./test_jconfigparser.exe
```

### 运行特定测试

```bash
cd build
./test_jconfigparser.exe -tc "Node 默认构造测试"
```

### 查看测试详情

```bash
cd build
./test_jconfigparser.exe -v
```

### 生成覆盖率报告

```bash
# 启用覆盖率重新编译
cmake -G "MinGW Makefiles" -B build_coverage -DJCONFIGPARSER_ENABLE_COVERAGE=ON
cmake --build build_coverage

# 运行测试生成覆盖率数据
./build_coverage/test_jconfigparser.exe

# 生成 gcov 覆盖率报告
cd C:/Users/esunny/Documents/code/JConfigParser
gcov -r build_coverage/CMakeFiles/test_jconfigparser.dir/ut/unittest.cpp.gcno

# 查看详细覆盖率
cat Node.h.gcov
```

## 测试覆盖率

- **行覆盖率**: 92.47% (Node.h)
- **分支覆盖率**: 98.89% (Node.h)
- **测试用例通过率**: 100% (67/67)
- **断言通过率**: 100% (328/328)

详细覆盖率报告见 [coverage_report.md](../build/coverage_report.md)

## 测试分类

### 1. 构造函数测试 (10 个测试用例)

测试各种构造方式：
- 默认构造
- createObject()
- createArray()
- 值构造（字符串、整数、浮点数、布尔值）
- 拷贝构造和移动构造
- 拷贝赋值和移动赋值
- clone()

### 2. 状态判断测试 (5 个测试用例)

测试节点状态查询：
- isValid() - 检查节点是否有效
- isNull() - 检查节点是否为 null
- isError() - 检查节点是否有错误
- getError() - 获取错误信息

### 3. 类型判断测试 (9 个测试用例)

测试各种类型的判断：
- isString(), isInt64(), isDouble(), isBool()
- isObject(), isArray(), isNumber()
- 模板方法 is<T>()

### 4. 取值测试 (3 个测试用例)

测试值获取功能：
- getValue<T>() - 获取值，类型不匹配返回默认值
- getValueOr<T>() - 获取值，自定义默认值

### 5. 获取子节点测试 (6 个测试用例)

测试子节点访问：
- get(key) - 获取对象子节点
- at(index) - 获取数组元素
- has(key) - 检查键是否存在
- remove(key) - 删除键
- size() - 获取大小
- keys() - 获取所有键名

### 6. 设置值测试 (8 个测试用例)

测试对象键值对设置：
- set(key, value) - 各种类型值
- set(key, Node) - Node 类型
- setObject(key), setArray(key) - 创建子对象/数组
- 覆盖测试

### 7. 数组追加测试 (8 个测试用例)

测试数组元素追加：
- append(value) - 各种类型
- append(Node) - Node 类型
- appendObject(), appendArray() - 追加子对象/数组
- 链式调用

### 8. 序列化测试 (4 个测试用例)

测试 JSON 序列化：
- toJson() - 紧凑格式和美化格式
- fromJson() - 解析成功和失败
- 循环测试

### 9. 边界测试 (5 个测试用例)

测试边界和特殊情况：
- 深层嵌套（3层以上）
- 大数组（1000元素）
- 大对象（1000键）
- 特殊字符（\n, \t, spaces）
- Unicode（中文、日文、韩文）

### 10. 错误处理测试 (10 个测试用例)

测试各种错误场景：
- 无效节点操作
- 非对象节点调用 get()
- 非数组节点调用 at()
- 索引越界
- 键不存在
- JSON 解析错误
- 链式调用中的错误传播

## 测试数据

### 测试输入示例

```cpp
// 简单对象
{"name": "Alice", "age": 30}

// 嵌套对象
{
  "config": {
    "database": {
      "host": "localhost",
      "port": 5432
    }
  }
}

// 数组
[1, 2, 3]

// 混合结构
{
  "version": "1.0.0",
  "servers": [
    {"host": "192.168.1.10", "port": 8080},
    {"host": "192.168.1.11", "port": 8080}
  ]
}
```

### 特殊情况测试

- 空对象 `{}`
- 空数组 `[]`
- null 值 `null`
- 空字符串 `""`
- 负数
- 大整数 `INT64_MAX`, `INT64_MIN`
- 浮点数 `3.14159`, `-2.5`
- 布尔值 `true`, `false`

## 高级特性测试

### 完美转发

测试 C++17 完美转发的正确性：

```cpp
// 左值传递
std::string str = "test";
obj.set("key", str);

// 右值传递
obj.set("key", std::string("value"));

// 链式调用
obj.set("a", 1).set("b", 2).set("c", 3);
```

### 内存安全

- 使用智能指针管理 RapidJSON Document
- 深拷贝避免悬挂指针
- 移动语义提升性能

### 错误传播

错误节点会传播到后续操作：

```cpp
Node invalid;
Node error = invalid.get("key");  // 返回错误节点
CHECK(!error.isValid());          // 无效
CHECK(error.isError());           // 有错误
```

## 添加新测试

### 测试模板

```cpp
TEST_CASE("测试用例描述") {
    // 准备
    Node obj = Node::createObject();

    // 执行
    obj.set("key", "value");

    // 断言
    CHECK(obj.isValid());
    CHECK(obj.get("key").getValue<std::string>() == "value");
}
```

### 测试最佳实践

1. **每个测试用例一个功能点**
2. **测试正常情况和边界情况**
3. **测试错误情况**
4. **使用有意义的测试用例名称**
5. **保持测试独立，不依赖执行顺序**

## 持续集成

在 CI/CD 中运行测试：

```bash
# 配置项目
cmake -B ci_build -DJCONFIGPARSER_BUILD_TESTS=ON
cmake --build ci_build

# 运行测试
./ci_build/test_jconfigparser.exe

# 检查返回值
if [ $? -eq 0 ]; then
    echo "所有测试通过"
else
    echo "测试失败"
    exit 1
fi
```

## 参考资料

- [doctest 文档](https://github.com/doctest/doctest)
- [RapidJSON 文档](http://rapidjson.org/)
- [CMake 文档](https://cmake.org/documentation/)

## 贡献

添加新功能时，请确保：

1. 编写相应的单元测试
2. 新测试通过
3. 原有测试仍然通过
4. 维护或提高代码覆盖率

---

**最后更新**: 2026年4月8日