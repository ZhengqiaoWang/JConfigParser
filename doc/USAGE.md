# JConfigParser 使用文档

## 目录

- [快速开始](#快速开始)
- [创建节点](#创建节点)
- [设置值](#设置值)
- [读取值](#读取值)
- [类型检查](#类型检查)
- [嵌套结构](#嵌套结构)
- [数组操作](#数组操作)
- [JSON 序列化](#json-序列化)
- [错误处理](#错误处理)
- [特殊类型](#特殊类型)
- [高级特性](#高级特性)
- [最佳实践](#最佳实践)
- [常见问题](#常见问题)

---

## 快速开始

### 最简单的例子

```cpp
#include "JConfigParser/Node.h"
#include <iostream>

int main() {
    // 创建 JSON 对象
    Node obj = Node::createObject();
    obj.set("name", "Alice");
    obj.set("age", 30);

    // 转换为 JSON 字符串
    std::cout << obj.toJson(true) << std::endl;

    // 从 JSON 字符串解析
    std::string json = R"({"name": "Bob", "age": 25})";
    Node parsed = Node::fromJson(json);

    // 读取值
    std::string name = parsed.get("name").getValue<std::string>();
    int age = parsed.get("age").getValue<int64_t>();

    std::cout << name << " is " << age << " years old" << std::endl;

    return 0;
}
```

---

## 创建节点

### 创建对象

```cpp
// 创建空对象
Node obj = Node::createObject();
```

### 创建数组

```cpp
// 创建空数组
Node arr = Node::createArray();
```

### 创建值节点

```cpp
// 字符串
Node str("Hello");
Node str2(std::string("World"));

// 数字
Node num(42);
Node num2(3.14);

// 布尔值
Node boolVal(true);

// null
Node nullVal = Node::fromJson("null");
```

---

## 设置值

### 设置基本值

```cpp
Node obj = Node::createObject();

// 字符串
obj.set("name", "Alice");
obj.set("city", "Beijing");

// 数字（各种整数类型）
obj.set("age", 30);
obj.set("score", 95);
obj.set("count", 1000LL);

// 浮点数
obj.set("price", 19.99);
obj.set("ratio", 0.75);

// 布尔值
obj.set("active", true);
obj.set("deleted", false);

// char 类型（转换为单字符字符串）
obj.set("grade", 'A');
obj.set("initial", 'B');
```

### 覆盖已存在的键

```cpp
Node obj = Node::createObject();
obj.set("value", 10);
obj.set("value", 20);  // 覆盖为 20

std::cout << obj.get("value").getValue<int64_t>();  // 输出: 20
```

### 链式调用

```cpp
Node obj = Node::createObject()
    .set("name", "Charlie")
    .set("age", 35)
    .set("active", true);
```

---

## 读取值

### 安全读取

```cpp
Node obj = Node::fromJson(R"({"name": "Alice", "age": 30})");

// 读取字符串
std::string name = obj.get("name").getValue<std::string>();

// 读取整数
int64_t age = obj.get("age").getValue<int64_t>();

// 读取浮点数
double price = obj.get("price").getValue<double>();

// 读取布尔值
bool active = obj.get("active").getValue<bool>();
```

### 使用默认值

```cpp
Node obj = Node::fromJson(R"({"name": "Alice"})");

// 键不存在时使用默认值
std::string name = obj.get("name").getValueOr("Unknown");      // "Alice"
std::string city = obj.get("city").getValueOr("Beijing");      // "Beijing"
int64_t age = obj.get("age").getValueOr(0);                     // 0
bool active = obj.get("active").getValueOr(false);             // false
```

### 类型不匹配

```cpp
Node strNode("hello");

// 从字符串读取整数会返回默认值 0
int64_t num = strNode.getValue<int64_t>();  // 0

// 从数字读取字符串会返回空字符串
Node numNode(42);
std::string str = numNode.getValue<std::string>();  // ""
```

---

## 类型检查

### is<T>() 方法

```cpp
Node node = Node::fromJson(R"({"name": "Alice", "age": 30, "active": true})");

// 检查类型
bool isString = node.get("name").is<std::string>();      // true
bool isInt = node.get("age").is<int64_t>();              // true
bool isBool = node.get("active").is<bool>();            // true

// char 类型检查（检查是否为字符串）
Node grade('A');
bool isChar = grade.is<char>();                           // true
bool isString = grade.is<std::string>();                  // true
bool isInt = grade.is<int64_t>();                         // false
```

### 基本类型检查

```cpp
Node obj = Node::createObject();
Node arr = Node::createArray();
Node str("hello");
Node num(42);
Node boolVal(true);
Node nullVal = Node::fromJson("null");

// 基本类型检查
bool isObject = obj.isObject();       // true
bool isArray = arr.isArray();         // true
bool isString = str.isString();       // true
bool isNumber = num.isNumber();       // true
bool isBool = boolVal.isBool();       // true
bool isNull = nullVal.isNull();       // true

// 检查是否有效
bool valid = obj.isValid();           // true
bool invalid = Node().isValid();      // false
```

---

## 嵌套结构

### 创建嵌套对象

```cpp
Node obj = Node::createObject();

// 创建嵌套对象
Node address = obj.setObject("address");
address.set("city", "Beijing");
address.set("country", "China");
address.set("zip", "100000");

// 访问嵌套数据
std::string city = obj.get("address").get("city").getValue<std::string>();
std::string zip = obj.get("address").get("zip").getValue<int64_t>();
```

### 创建嵌套数组

```cpp
Node obj = Node::createObject();

// 创建嵌套数组
Node hobbies = obj.setArray("hobbies");
hobbies.append("coding");
hobbies.append("reading");
hobbies.append("gaming");

// 访问数组元素
std::string hobby1 = obj.get("hobbies").at(0).getValue<std::string>();
std::string hobby2 = obj.get("hobbies").at(1).getValue<std::string>();
```

### 复杂嵌套

```cpp
Node obj = Node::createObject();

// 创建多层嵌套
Node data = obj.setObject("data");
Node user = data.setObject("user");
user.set("name", "Alice");
user.set("email", "alice@example.com");

// 访问
std::string name = obj.get("data").get("user").get("name").getValue<std::string>();

/* JSON 输出:
{
  "data": {
    "user": {
      "name": "Alice",
      "email": "alice@example.com"
    }
  }
}
*/
```

---

## 数组操作

### 创建和添加元素

```cpp
// 创建数组
Node arr = Node::createArray();

// 添加各种类型元素
arr.append(1);
arr.append("hello");
arr.append(3.14);
arr.append(true);
arr.append('A');  // char 转换为 "A"

/* JSON: [1, "hello", 3.14, true, "A"] */
```

### 访问数组元素

```cpp
Node arr = Node::fromJson(R"([1, 2, 3, 4, 5])");

// 通过索引访问
Node first = arr.at(0);    // 1
Node third = arr.at(2);    // 3
Node last = arr.at(4);     // 5

// 获取值
int64_t val = arr.at(0).getValue<int64_t>();  // 1
```

### 获取数组长度

```cpp
Node arr = Node::fromJson(R"([1, 2, 3]);
// 1
```

### 数组越界

```cpp
Node arr = Node::fromJson(R"([1, 2, 3])");

// 访问越界索引返回错误节点
Node outOfBounds = arr.at(10);
if (outOfBounds.isError()) {
    std::cout << "Index out of bounds" << std::endl;
}
```

---

## JSON 序列化

### 转换为 JSON 字符串

```cpp
Node obj = Node::createObject();
obj.set("name", "Alice");
obj.set("age", 30);

// 紧凑格式
std::string compact = obj.toJson(false);
// {"name":"Alice","age":30}

// 格式化输出
std::string formatted = obj.toJson(true);
// {
//   "name": "Alice",
//   "age": 30
// }
```

### 解析 JSON 字符串

```cpp
// 解析对象
std::string json1 = R"({"name": "Alice", "age": 30})";
Node obj = Node::fromJson(json1);

// 解析数组
std::string json2 = R"([1, 2, 3])";
Node arr = Node::fromJson(json2);

// 解析字符串
std::string json3 = R"("hello")";
Node str = Node::fromJson(json3);

// 解析 null
std::string json4 = "null";
Node nullVal = Node::fromJson(json4);
```

### 处理无效 JSON

```cpp
std::string invalidJson = "{bad json}";
Node node = Node::fromJson(invalidJson);

if (node.isError()) {
    std::cout << "Invalid JSON: " << node.getError() << std::endl;
}
```

---

## 错误处理

### 错误类型

```cpp
Node obj = Node::createObject();

// 1. 键不存在
 Node missing = obj.get("nonexistent");
// missing.isError() == true

// 2. 非对象类型访问
Node arr = Node::createArray();
Node err = arr.get("key");
// err.isError() == true

// 3. 数组越界
Node arr2 = Node::fromJson(R"([1, 2])");
Node outOfBounds = arr2.at(10);
// outOfBounds.isError() == true

// 4. 无效 JSON
Node invalid = Node::fromJson("{invalid json}");
// invalid.isError() == true
```

### 安全的值访问

```cpp
Node obj = Node::createObject();

// 方法1：先检查是否存在
if (obj.has("name")) {
    std::string name = obj.get("name").getValue<std::string>();
}

// 方法2：使用默认值
std::string name = obj.get("name").getValueOr("Unknown");

// 方法3：检查错误
Node nameNode = obj.get("name");
if (nameNode.isError()) {
    std::cout << "Key not found: " << nameNode.getError() << std::endl;
} else {
    std::string name = nameNode.getValue<std::string>();
}
```

### 错误信息

```cpp
Node obj = Node::createObject();
Node err = obj.get("missing");

if (err.isError()) {
    std::cout << "Error: " << err.getError() << std::endl;
    // 输出: Error: Key 'missing' not found in object
}
```

---

## 特殊类型

### char 类型

在 JSON 中没有 char 类型，JConfigParser 将 char 转换为单字符字符串。

#### 创建 char 节点

```cpp
// char 转换为单字符字符串
Node c('A');
std::cout << c.toJson(false);  // "A"

// signed char
Node sc(static_cast<signed char>('B'));
std::cout << sc.toJson(false);  // "B"

// unsigned char
Node uc(static_cast<unsigned char>('C'));
std::cout << uc.toJson(false);  // "C"
```

#### 读取 char 值

```cpp
Node str("Hello");

// 从字符串取第一个字符
char c = str.getValue<char>();  // 'H'

// 从首字母缩写提取
Node abbr("NASA");
char first = abbr.getValue<char>();  // 'N'
```

#### char 类型的安全处理

```cpp
// 空字符串返回 '\0'
Node empty("");
char c1 = empty.getValue<char();  // '\0'

// 非字符串返回 '\0'
Node num(42);
char c2 = num.getValue<char>();  // '\0'

// 无效节点返回 '\0'
Node invalid;
char c3 = invalid.getValue<char>();  // '\0'

// 使用默认值
char safe = empty.getValueOr<char>('X');  // 'X'
```

### nullptr

```cpp
// 创建 null 节点
Node nullNode = Node::fromJson("null");
std::cout << nullNode.toJson(false);  // "null"

// 检查是否为 null
if (nullNode.isNull()) {
    std::cout << "It's null" << std::endl;
}
```

---

## 高级特性

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

### 克隆子节点

```cpp
Node root = Node::createObject();
Node sub = root.setObject("nested");
sub.set("key", "value");

// 克隆子节点（只克隆子节点，不包含根节点其他内容）
Node subRef = root.get("nested");
Node cloned = subRef.clone();

/* cloned 包含:
{
  "key": "value"
}
*/
```

### 迭代对象键

```cpp
Node obj = Node::fromJson(R"({"name":"Alice","age":30,"city":"Beijing"})");

// 获取所有键
auto keys = obj.keys();
for (const auto& key : keys) {
    std::cout << key << std::endl;
}
// 输出: name, age, city
```

### 迭代数组元素

```cpp
Node arr = Node::fromJson(R"([1, 2, 3, 4, 5])");

// 通过索引遍历
for (size_t i = 0; i < arr.size(); i++) {
    int64_t val = arr.at(i).getValue<int64_t>();
    std::cout << val << " ";
}
// 输出: 1 2 3 4 5
```

---

## 最佳实践

### 1. 类型安全

始终使用 `is<T>()` 检查类型后再读取：

```cpp
// ✅ 好的做法
if (node.is<int64_t>()) {
    int64_t value = node.getValue<int64_t>();
}

// ❌ 避免：可能返回默认值0
int64_t value = node.getValue<int64_t>();
```

### 2. 使用默认值

对于可选字段，使用 `getValueOr()`：

```cpp
// ✅ 好的做法
std::string name = obj.get("name").getValueOr("Unknown");
int64_t age = obj.get("age").getValueOr(0);

// ❌ 避免：可能崩溃或返回空值
std::string name = obj.get("name").getValue<std::string>();
```

### 3. 检查错误

始终检查操作是否成功：

```cpp
// ✅ 好的做法
Node node = obj.get("key");
if (node.isError()) {
    std::cout << "Error: " << node.getError() << std::endl;
    // 处理错误
}

// ❌ 避免：可能访问无效节点
std::string value = obj.get("key").getValue<std::string>();
```

### 4. 链式调用

使用链式调用使代码更简洁：

```cpp
// ✅ 好的做法
Node obj = Node::createObject()
    .set("name", "Alice")
    .set("age", 30)
    .set("active", true);

// ❌ 避免：多次调用
Node obj = Node::createObject();
obj.set("name", "Alice");
obj.set("age", 30);
obj.set("active", true);
```

### 5. 使用引用避免拷贝

对于大对象，使用引用传递：

```cpp
// ✅ 好的做法
std::string getJson(const Node& node) {
    return node.toJson(true);
}

// ❌ 避免：会产生拷贝
std::string getJson(Node node) {
    return node.toJson(true);
}
```

---

## 常见问题

### Q: 如何检查键是否存在？

```cpp
Node obj = Node::fromJson(R"({"name": "Alice"})");

if (obj.has("name")) {
    std::cout << "Key exists" << std::endl;
}
```

### Q: 如何删除对象中的键？

```cpp
Node obj = Node::fromJson(R"({"name": "Alice", "age": 30})");

int64_t age = obj.get("age").getValue<int64_t>();
```

### Q: 如何合并两个 JSON 对象？

```cpp
Node obj1 = Node::fromJson(R"({"name": "Alice"})");
Node obj2 = Node::fromJson(R"({"age": 30})");

// 复制键值对
auto keys2 = obj2.keys();
for (const auto& key : keys2) {
    // 获取 obj2 的值并设置到 obj1
    if (obj2.get(key).is<std::string>()) {
        obj1.set(key, obj2.get(key).getValue<std::string>());
    } else if (obj2.get(key).is<int64_t>()) {
        obj1.set(key, obj2.get(key).getValue<int64_t>());
    }
    // 其他类型...
}
```

### Q: char 为什么转换为字符串？

JSON 标准中没有 char 类型，只有字符串类型。将 char 转换为单字符字符串：
- 符合 JSON 标准
- 提高可读性（`"A"` vs `65`）
- 支持从字符串提取首字符

详见 [char 类型设计文档](CHAR_TYPE_DESIGN.md)。

### Q: 如何处理大型 JSON 文件？

对于大型 JSON 文件，建议：
1. 使用流式解析（快速解析，内存高效）
2. 解析后立即处理，避免保留全部数据
3. 使用 `clone()` 分离需要的部分

```cpp
// 只保留需要的部分
Node full = Node::fromJson(largeJsonString);
Node needed = full.get("data").clone();  // 克隆需要的部分
// 现在 full 可以被销毁，只保留 needed
```

---

## 更多示例

查看 `example/` 目录获取更多实用示例：
- `basic.cpp` - 基础用法
- `nested.cpp` - 嵌套结构
- `serialization.cpp` - 序列化
- `error_handling.cpp` - 错误处理

---

**文档版本**: 1.0.0
**最后更新**: 2026-04-08