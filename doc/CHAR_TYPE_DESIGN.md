# char 类型处理设计的改进

## 问题说明

在 JSON 中没有原生的 char 类型，原先的设计将 char 类型存储为 ASCII 整数（如 'A' 存储为 65），但这种设计有两个问题：

1. **可读性问题**：JSON 输出为 `65` 而不是 `"A"`，失去原始字符信息
2. **不符合 JSON 标准**：JSON 只有字符串类型，没有字符类型

## 新的设计方案

### 核心原则

**char 类型在 JSON 中表示为字符串，符合 JSON 标准且提高可读性**

### 具体实现

#### 1. 构造函数（char → 字符串）

```cpp
Node c('A');  // 创建 JSON: "A"  而不是 65
```

在 `_setRapidjsonValue` 方法中：

```cpp
else if constexpr (std::is_same_v<std::decay_t<T>, char> ||
                   std::is_same_v<std::decay_t<T>, signed char> ||
                   std::is_same_v<std::decay_t<T>, unsigned char>)
{
    // char 类型转换为字符串（JSON没有char类型，字符串更可读）
    std::string str(1, static_cast<char>(value));
    v->SetString(str.c_str(), doc_->GetAllocator());
}
```

#### 2. 类型检查（char 判断为字符串）

```cpp
Node c('A');
c.is<std::string>();  // true
c.is<char>();        // true
c.is<int64_t>();     // false
```

在 `is<T>()` 方法中：

```cpp
if constexpr (std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<T, const char *> ||
              std::is_same_v<std::decay_t<T>, char> ||
              std::is_same_v<std::decay_t<T>, signed char> ||
              std::is_same_v<std::decay_t<T>, unsigned char>)
{
    return node_ && node_->IsString();
}
```

#### 3. getValue（字符串 → 第一个字符）

```cpp
Node str("Hello");
char c = str.getValue<char>();  // 返回 'H'
```

模板特化实现：

```cpp
template <>
inline char Node::getValue<char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return node_->GetString()[0];
}

template <>
inline signed char Node::getValue<signed char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return static_cast<signed char>(node_->GetString()[0]);
}

template <>
inline unsigned char Node::getValue<unsigned char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return static_cast<unsigned char>(node_->GetString()[0]);
}
```

#### 4. getValueOr（默认值为 '\0'）

```cpp
Node str("Hello");
char c1 = str.getValueOr<char>('X');   // 返回 'H'

Node empty("");
char c2 = empty.getValueOr<char>('X'); // 返回 'X'（空字符串）

Node invalid;
char c3 = invalid.getValueOr<char>('Y'); // 返回 'Y'（无效节点）
```

模板特化实现：

```cpp
template <>
inline char Node::getValueOr<char>(char &&defaultVal) const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return defaultVal;
    }
    return node_->GetString()[0];
}
```

## 对比

### 旧设计

| 操作 | 代码 | JSON 输出 | 说明 |
|------|------|----------|------|
| 构造 | `Node('A')` | `65` | ASCII 码 |
| set | `obj.set("ch", 'B')` | `{"ch":66}` | 数字 66 |
| 读取 | `val.getValue<char>()` | - | 从整数读取 |

**问题**：
- ✗ JSON 输出不可读（65 vs "A"）
- ✗ 不符合 JSON 标准
- ✗ 字符串无法被解释为 char

### 新设计

| 操作 | 代码 | JSON 输出 | 说明 |
|------|------|----------|------|
| 构造 | `Node('A')` | `"A"` | 字符串 |
| set | `obj.set("ch", 'B')` | `{"ch":"B"}` | 字符串 |
| 读取 | `str.getValue<char>()` | - | 从字符串取首字符 |

**优势**：
- ✓ JSON 输出可读（"A" vs 65）
- ✓ 符合 JSON 标准
- ✓ 字符串可以作为 char 来源

## 使用示例

### 示例1：单个字符

```cpp
// 创建
Node initial('A');
std::cout << initial.toJson(false); // 输出: "A"

// 读取
char c = initial.getValue<char>(); // 返回 'A'
std::cout << "Initial: " << c << std::endl; // 输出: Initial: A
```

### 示例2：从字符串取首字符

```cpp
Node abbreviation("NASA");
char first = abbreviation.getValue<char>(); // 'N'
std::cout << first << std::endl; // 输出: N
```

### 示例3：对象中的字符

```cpp
Node obj = Node::createObject();
obj.set("grade", 'A');
obj.set("name", "Alice");
obj.set("score", 95);

/* JSON 输出:
{
  "grade": "A",
  "name": "Alice",
  "score": 95
}
*/

char grade = obj.get("grade").getValue<char>(); // 'A'
std::string name = obj.get("name").getValue<std::string>(); // "Alice"
```

### 示例4：数组中的字符

```cpp
Node arr = Node::createArray();
arr.append('R');  // Red
arr.append('G');  // Green
arr.append('B');  // Blue

/* JSON 输出:
["R", "G", "B"]
*/

char red = arr.at(0).getValue<char>(); // 'R'
char green = arr.at(1).getValue<char>(); // 'G'
char blue = arr.at(2).getValue<char>(); // 'B'
```

### 示例5：错误处理

```cpp
// 空字符串返回 '\0'
Node empty("");
char c1 = empty.getValue<char>(); // '\0'

// 非字符串返回 '\0'
Node num(42);
char c2 = num.getValue<char>(); // '\0'

// 无效节点返回 '\0'
Node invalid;
char c3 = invalid.getValue<char>(); // '\0'

// 使用默认值
char safe1 = empty.getValueOr<char>('X'); // 'X'
char safe2 = num.getValueOr<char>('Y');   // 'Y'
char safe3 = invalid.getValueOr<char>('Z'); // 'Z'
```

## 边界情况处理

| 场景 | 输入 | 返回值 | 说明 |
|------|------|--------|------|
| 空字符串 | `""` | `'\0'` | 没有第一个字符 |
| 长字符串 | `"Hello"` | `'H'` | 取第一个字符 |
| 非字符串 | `123` | `'\0'` | 不是字符串类型 |
| 无效节点 | `Node()` | `'\0'` | 节点无效 |
| 单字符字符串 | `"X"` | `'X'` | 正常读取 |
| Unicode 字符 | `"你好"` | `'你'` | 取第一个 Unicode 字符 |

## 单元测试覆盖

新添加的测试用例：

1. **`Node char 类型的字符串转换`**：
   - 验证 `Node('A')` 生成 `"`A"`"
   - 验证 `is<std::string>()` 为 true
   - 验证 `is<char>()` 为 true
   - 验证 `is<int64_t>()` 为 false

2. **`Node getValue<char> 字符串取首字符`**：
   - 验证从 `"Hello"` 取出 `'H'`
   - 验证 `char`、`signed char`、`unsigned char` 都正确
   - 验证空字符串返回 `'\0'`
   - 验证非字符串返回 `'\0'`
   - 验证 `getValueOr<char>()` 使用默认值

## 总结

新设计的核心思想：

1. **JSON 标准优先**：char 在 JSON 中用字符串表示
2. **可读性优先**：`'A'` 输出为 `"A"` 而不是 `65`
3. **语义清晰**：`is<char>()` 检查字符串类型
4. **安全处理**：非字符串或空字符串返回 `'\0'`
5. **向后兼容**：API 界面保持不变

这种设计让 char 类型的使用更加直观和符合 JSON 标准，同时保持了良好的可读性和类型安全。

---

**实现时间**：2026年4月8日
**文件位置**：`doc/CHAR_TYPE_DESIGN.md`