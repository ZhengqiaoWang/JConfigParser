# JConfigParser

[JConfigParser](https://github.com/ZhengqiaoWang/JConfigParser) 是一个基于 RapidJSON 的现代 C++17 JSON 解析库，提供类型安全、易用且高性能的 JSON 操作接口。

## 特性

- ✅ **Type Safe**: 使用 C++17 模板提供类型安全的 API
- 🚀 **High Performance**: 基于 RapidJSON，零拷贝设计
- 🎯 **Easy to Use**: 直观的接口，支持链式调用
- 🔒 **Safe by Default**: 完善的错误处理，避免异常崩溃
- 📚 **Rich Features**: 支持对象、数组、字符串、数字、布尔值、null
- 🧪 **Well Tested**: 89+ 测试用例，97%+ 代码覆盖率

## 支持的类型

| C++ 类型 | JSON 类型 | 说明 |
|---------|-----------|------|
| `std::string` / `const char*` | string | 字符串 |
| `char` / `signed char` / `unsigned char` | string | 单字符字符串（JSON 标准不包含 char 类型） |
| `int` / `unsigned int` / `int32_t` / `uint32_t` / `int64_t` / `uint64_t` | number | 整数 |
| `double` / `float` | number | 浮点数 |
| `bool` | boolean | 布尔值 |
| `nullptr` | null | 空值 |

## 代码示例

### 基础用法

```cpp
#include "JConfigParser/Node.h"

// 创建对象
Node obj = Node::createObject();
obj.set("name", "Alice");
obj.set("age", 30);
obj.set("active", true);

// 创建数组
Node arr = Node::createArray();
arr.append("item1");
arr.append(42);
arr.append(3.14);

// 输出 JSON
std::cout << obj.toJson(true);  // 格式化输出
// {
//   "name": "Alice",
//   "age": 30,
//   "active": true
// }
```

### 解析 JSON

```cpp
// 从字符串解析
std::string jsonStr = R"({"name": "Bob", "age": 25})";
Node node = Node::fromJson(jsonStr);

// 读取数据
std::string name = node.get("name").getValue<std::string>();
int age = node.get("age").getValue<int64_t>();
```

### 链式调用

```cpp
Node obj = Node::createObject()
    .set("name", "Charlie")
    .set("age", 35)
    .set("active", true);

Node arr = Node::createArray()
    .append(1)
    .append(2)
    .append(3);
```

### 嵌套结构

```cpp
Node obj = Node::createObject();

// 创建嵌套对象
Node address = obj.setObject("address");
address.set("city", "Beijing");
address.set("country", "China");

// 创建嵌套数组
Node hobbies = obj.setArray("hobbies");
hobbies.append("coding");
hobbies.append("reading");
hobbies.append("gaming");

// 访问嵌套数据
std::string city = obj.get("address").get("city").getValue<std::string>();
```

### 错误处理

```cpp
Node obj = Node::createObject();

// 安全访问：键不存在时返回错误节点
Node nameNode = obj.get("name");
if (nameNode.isError()) {
    std::cout << "Key not found" << std::endl;
}

// 使用默认值：键不存在或类型不匹配时返回默认值
std::string name = obj.get("name").getValueOr("Unknown");

// 类型检查
if (numNode.is<int64_t>()) {
    int64_t value = numNode.getValue<int64_t>();
}

// 错误信息
if (node.isError()) {
    std::cout << "Error: " << node.getError() << std::endl;
}
```

### 特殊类型：char

```cpp
// char 类型在 JSON 中表示为单字符字符串
Node grade('A');
std::cout << grade.toJson(false);  // 输出: "A"

// 从字符串取第一个字符
Node str("Hello");
char c = str.getValue<char>();  // 返回 'H'

// 使用默认值
char initial = str.getValueOr<char>('X');  // 返回 'H'
char empty = Node("").getValueOr<char>('X');  // 返回 'X'（空字符串）
```

### 克隆节点

```cpp
Node original = Node::createObject();
original.set("name", "Alice");
original.set("value", 42);

// 深拷贝
Node cloned = original.clone();

// 修改原始对象不影响克隆对象
original.set("value", 100);
std::cout << cloned.get("value").getValue<int64_t>();  // 仍然输出 42
```

## 编译

### 要求

- C++17 或更高版本
- CMake 3.10+
- 支持 C++17 的编译器（GCC 7+, Clang 5+, MSVC 2017+）

### 编译步骤

```bash
# 克隆仓库
git clone https://github.com/ZhengqiaoWang/JConfigParser.git
cd JConfigParser

# 使用 CMake 构建
mkdir build && cd build
cmake ..
cmake --build .

# 运行测试
ctest
# 或
./test_jconfigparser
```

### 集成到项目

#### 方法1：直接复制头文件

将 `include/JConfigParser/` 目录复制到你的项目中，包含头文件即可：

```cmake
# CMakeLists.txt
target_include_directories(your_target PRIVATE include)
```

#### 方法2：使用 CMake

```cmake
# CMakeLists.txt
add_subdirectory(JConfigParser)
target_link_libraries(your_target PRIVATE JConfigParser)
target_include_directories(your_target PRIVATE ${CMAKE_SOURCE_DIR}/JConfigParser/include)
```

## 测试

```bash
# 编译并运行所有测试
cd build
cmake --build .
./test_jconfigparser

# 运行特定测试
./test_jconfigparser --test-case="Test Case Name"

# 生成覆盖率报告
cmake -DJCONFIGPARSER_ENABLE_COVERAGE=ON ..
cmake --build .
./test_jconfigparser
gcov build/CMakeFiles/test_jconfigparser.dir/ut/unittest.cpp.gcno
```

## 性能

JConfigParser 基于 RapidJSON，具有优秀的性能表现：

- 🚀 **零拷贝设计**：避免不必要的内存拷贝
- ⚡ **SAX/DOM 双模式**：RapidJSON 原生支持
- 📦 **紧凑内存布局**：优化内存使用


## 文档

- [使用文档](doc/USAGE.md) - 详细的使用指南和 API 参考
- [使用示例](example/) - 更多实用示例

## 贡献

欢迎贡献！请提交 Pull Request 或 Issue。

## License

MIT License - 详见 [LICENSE](LICENSE) 文件

## 致谢

本项目基于 [RapidJSON](https://github.com/Tencent/rapidjson/) 开发，感谢 RapidJSON 项目的贡献。

使用 [doctest](https://github.com/onqtam/doctest) 进行单元测试。

## 更新日志

### v1.0.0 (2026-04-08)

- ✨ 首次发布
- ✅ 完整的 JSON 解析和生成功能
- 🎯 类型安全的模板 API
- 🔒 完善的错误处理机制
- 🧪 89+ 测试用例，97%+ 代码覆盖率
- 📚 完整的文档和示例
- 🔧 char 类型支持（转换为单字符字符串）
- 🚀 clone() 子节点克隆支持

## 联系方式

- 项目主页: https://github.com/ZhengqiaoWang/JConfigParser
- 问题反馈: https://github.com/ZhengqiaoWang/JConfigParser/issues

---

**Enjoy using JConfigParser!** 🎉