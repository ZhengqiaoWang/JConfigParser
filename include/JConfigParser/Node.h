/**
 * @file Node.h
 * @brief JConfigParser JSON 库头文件
 * @details 基于 RapidJSON 实现的 C++17 JSON 库，提供类型安全且易用的接口
 * @author JConfigParser Team
 * @date 2026
 */

#ifndef NODE_H
#define NODE_H

// 启动rapidjson支持
#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_HAS_CXX11 1

#include <algorithm>
#include <climits>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @class Node
 * @brief JSON 节点类，封装 RapidJSON 功能
 * @details 提供类型安全、易用的 JSON 操作接口，支持对象、数组、字符串、数字、布尔值等类型
 *
 * 主要特性：
 * - 使用 C++17 模板和完美转发，避免不必要的拷贝
 * - 支持链式调用
 * - 提供错误传播机制
 * - 完整的类型检查和类型转换
 */
namespace ConfigParser
{

class Node
{
private:
    std::shared_ptr<rapidjson::Document> doc_; /// RapidJSON 文档（用于管理内存）
    rapidjson::Value *node_;                   /// 指向当前节点
    std::string error_;                        /// 错误信息（如果有）

    // 私有构造:从已有 Document 和节点创建
    Node(std::shared_ptr<rapidjson::Document> doc, rapidjson::Value *node)
        : doc_(doc), node_(node) {}

    // 私有构造:创建错误节点
    static Node createError(const std::string &errorMsg)
    {
        Node node;
        node.error_ = errorMsg;
        node.doc_ = nullptr;
        node.node_ = nullptr;
        return node;
    }

    // ==================== 内部辅助函数 ====================

    /**
     * @brief 创建单个值的Document（用于构造函数）
     * @tparam T 值类型
     * @param node 目标节点
     * @param value 要设置的值（使用完美转发）
     */
    template <typename T>
    static void _createValueDocument(Node &node, T &&value)
    {
        node.doc_ = std::make_shared<rapidjson::Document>();

        _setRapidjsonValue(node.doc_, node.doc_.get(), std::forward<T>(value));

        node.node_ = node.doc_.get();
    }

    /**
     * @brief 创建rapidjson::Value（用于set和append）
     * @tparam T 值类型
     * @param value 要设置的值（使用完美转发）
     * @return rapidjson::Value 对象
     */
    template <typename T>
    static void _setRapidjsonValue(std::shared_ptr<rapidjson::Document> &doc_, rapidjson::Value *v, T &&value)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        {
            v->SetBool(value);
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, char> ||
                           std::is_same_v<std::decay_t<T>, signed char> ||
                           std::is_same_v<std::decay_t<T>, unsigned char>)
        {
            // char 类型转换为字符串（JSON没有char类型，字符串更可读）
            std::string str(1, static_cast<char>(value));
            v->SetString(str.c_str(), doc_->GetAllocator());
        }
        else if constexpr (std::is_constructible_v<std::string, std::decay_t<T>> ||
                           std::is_same_v<T, const char *>)
        {
            std::string str(std::forward<T>(value));
            v->SetString(str.c_str(), doc_->GetAllocator());
        }
        else if constexpr (std::is_floating_point_v<std::decay_t<T>>)
        {
            v->SetDouble(std::forward<std::decay_t<T>>(value));
        }
        else if constexpr (std::is_integral_v<std::decay_t<T>>)
        {
            // 整数
            if constexpr (sizeof(T) == 2 || sizeof(T) == 4)
            {
                // 16 bit / 32 bit
                if constexpr (std::is_unsigned_v<std::decay_t<T>>)
                {
                    v->SetUint(std::forward<std::decay_t<T>>(value));
                }
                else
                {
                    v->SetInt(std::forward<std::decay_t<T>>(value));
                }
            }
            else if constexpr (sizeof(T) == 8)
            {
                // 64 bit
                if constexpr (std::is_unsigned_v<std::decay_t<T>>)
                {
                    v->SetUint64(std::forward<std::decay_t<T>>(value));
                }
                else
                {
                    v->SetInt64(std::forward<std::decay_t<T>>(value));
                }
            }
        }
    }

    /**
     * @brief 通用的set方法实现
     * @tparam T 值类型
     * @param key 键名
     * @param value 键值（使用完美转发）
     * @return Node& 自身引用，支持链式调用
     */
    template <typename T>
    Node &_setImpl(const std::string &key, T &&value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        // 检查键是否已存在，如果存在则覆盖
        auto it = node_->FindMember(key.c_str());
        if (it != node_->MemberEnd())
        {
            // 键已存在，直接修改值（避免RemoveMember+AddMember的开销）
            _setRapidjsonValue(doc_, &it->value, std::forward<T>(value));
            return *this;
        }

        // 键不存在，添加新的键值对
        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        _setRapidjsonValue(doc_, &v, std::forward<T>(value));
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    /**
     * @brief 通用的append方法实现
     * @tparam T 值类型
     * @param value 要添加的值（使用完美转发）
     * @return Node& 自身引用，支持链式调用
     */
    template <typename T>
    Node &_appendImpl(T &&value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v;
        _setRapidjsonValue(doc_, &v, std::forward<T>(value));
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

public:
    // ==================== 构造 ====================

    /**
     * @brief 默认构造函数
     * @details 创建一个无效的 null 节点
     */
    Node() : doc_(nullptr), node_(nullptr) {}

    /**
     * @brief 创建空对象
     * @return Node 对象节点
     * @details 创建后可以使用 set() 方法添加键值对
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("name", "Alice").set("age", 30);
     * @endcode
     */
    static Node createObject()
    {
        Node obj;
        obj.doc_ = std::make_shared<rapidjson::Document>();
        obj.doc_->SetObject();
        obj.node_ = obj.doc_.get();
        return obj;
    }

    /**
     * @brief 创建空数组
     * @return Node 数组节点
     * @details 创建后可以使用 append() 方法添加元素
     *
     * 示例：
     * @code
     * Node arr = Node::createArray();
     * arr.append(1).append(2).append(3);
     * @endcode
     */
    static Node createArray()
    {
        Node arr;
        arr.doc_ = std::make_shared<rapidjson::Document>();
        arr.doc_->SetArray();
        arr.node_ = arr.doc_.get();
        return arr;
    }

    /**
     * @brief 值构造函数 - 支持多种类型
     * @tparam T 值类型，支持 std::string, int, int64_t, double, bool 等
     * @param val 要包装的值
     * @details 使用完美转发，支持左值和右值
     *
     * 示例：
     * @code
     * Node str("hello");           // 字符串
     * Node num(42);                // 整数
     * Node d(3.14);                // 浮点数
     * Node b(true);                // 布尔值
     * Node str2(std::string("test")); // std::string右值
     * @endcode
     */
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, Node> &&
                              (std::is_constructible_v<std::string, std::decay_t<T>> ||
                               std::is_integral_v<std::decay_t<T>> ||
                               std::is_floating_point_v<std::decay_t<T>>)>>
    Node(T &&val)
    {
        _createValueDocument(*this, std::forward<T>(val));
    }

    /**
     * @brief 拷贝构造函数
     * @param other 要拷贝的节点
     * @details 执行深拷贝，确保内存安全
     */
    Node(const Node &other)
        : doc_(other.doc_), node_(other.node_), error_(other.error_)
    {
        if (other.doc_ && other.node_ == other.doc_.get())
        {
            doc_ = std::make_shared<rapidjson::Document>();
            doc_->CopyFrom(*other.doc_, doc_->GetAllocator());
            node_ = doc_.get();
        }
    }

    /**
     * @brief 移动构造函数
     * @param other 要移动的节点
     * @details 接管 other 的资源，other 变为无效节点
     */
    Node(Node &&other) noexcept
        : doc_(std::move(other.doc_)), node_(other.node_), error_(std::move(other.error_))
    {
        other.node_ = nullptr;
    }

    // ==================== 拷贝与克隆 ====================

    /**
     * @brief 拷贝赋值运算符
     * @param other 要拷贝的节点
     * @return Node& 自身引用
     * @details 执行深拷贝
     */
    Node &operator=(const Node &other)
    {
        if (this != &other)
        {
            doc_ = other.doc_;
            node_ = other.node_;
            error_ = other.error_;

            if (other.doc_ && other.node_ == other.doc_.get())
            {
                doc_ = std::make_shared<rapidjson::Document>();
                doc_->CopyFrom(*other.doc_, doc_->GetAllocator());
                node_ = doc_.get();
            }
        }
        return *this;
    }

    /**
     * @brief 移动赋值运算符
     * @param other 要移动的节点
     * @return Node& 自身引用
     * @details 接管 other 的资源
     */
    Node &operator=(Node &&other) noexcept
    {
        if (this != &other)
        {
            doc_ = std::move(other.doc_);
            node_ = other.node_;
            error_ = std::move(other.error_);
            other.node_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 克隆当前节点
     * @return Node 克隆的节点
     * @details 执行深拷贝，返回一个完全独立的副本
     *
     * 行为：
     * - 根节点（node_ == doc_）：克隆整个文档
     * - 子节点（node_ != doc_）：将子节点复制到新文档的根节点
     *
     * 返回的节点拥有独立的文档，不依赖原节点的内存
     */
    Node clone() const
    {
        if (!doc_)
        {
            return Node();
        }

        Node cloned;
        cloned.doc_ = std::make_shared<rapidjson::Document>();

        if (node_ == doc_.get())
        {
            // 克隆根节点：复制整个文档
            cloned.doc_->CopyFrom(*doc_, cloned.doc_->GetAllocator());
            cloned.node_ = cloned.doc_.get();
        }
        else
        {
            // 克隆子节点：将子节点复制到新文档的根节点
            cloned.doc_->CopyFrom(*node_, cloned.doc_->GetAllocator());
            cloned.node_ = cloned.doc_.get();
        }

        return cloned;
    }

    // ==================== 状态判断 ====================

    /**
     * @brief 检查节点是否有效
     * @return bool 节点有效返回 true，否则返回 false
     * @details 无效情况包括：
     *   - 未初始化（默认构造）
     *   - 已被移动（移动后的源对象）
     *   - 操作失败（键不存在、索引越界、解析错误等）
     */
    bool isValid() const
    {
        // 节点必须指向有效的文档和值，且没有错误
        return doc_ != nullptr && node_ != nullptr && error_.empty();
    }

    /**
     * @brief bool 转换运算符（显式）
     * @return bool 节点有效返回 true，否则返回 false
     * @details 作用等同于 isValid()，提供更简洁的写法
     *
     * 使用示例：
     * @code
     *   Node node = obj.get("key");
     *   if (!node) {  // 等同于 if (!node.isValid())
     *       std::cerr << node.getError() << std::endl;
     *   }
     * @endcode
     *
     * @note 使用 explicit 关键字防止隐式转换（如与整数比较）
     */
    explicit operator bool() const
    {
        return isValid();
    }

    /**
     * @brief 检查节点是否为 null
     * @return bool 节点为 null 返回 true，否则返回 false
     */
    bool isNull() const
    {
        return node_ && node_->IsNull();
    }

    /**
     * @brief 获取错误信息
     * @return std::string 错误信息字符串
     * @details 如果没有错误则返回空字符串
     * @note 通常配合 isValid() 使用：`if (!node.isValid()) { cout << node.getError(); }`
     */
    std::string getError() const
    {
        return error_;
    }

    // ==================== 类型判断 ====================

    /**
     * @brief 检查节点类型（模板方法）
     * @tparam T 要检查的类型
     * @return bool 类型匹配返回 true，否则返回 false
     *
     * 支持的类型：
     * - std::string / const char* / char / signed char / unsigned char
     * - int64_t / uint64_t
     * - double
     * - bool
     *
     * 注意：char/signed char/unsigned char 在 JSON 中表示为单字符字符串
     */
    template <typename T>
    bool is() const
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<T, const char *> ||
                      std::is_same_v<std::decay_t<T>, char> ||
                      std::is_same_v<std::decay_t<T>, signed char> ||
                      std::is_same_v<std::decay_t<T>, unsigned char>)
        {
            return node_ && node_->IsString();
        }
        else
        {
            return node_ && node_->Is<std::decay_t<T>>();
        }
    }

    /**
     * @brief 检查是否为对象
     * @return bool 是对象返回 true，否则返回 false
     */
    bool isObject() const { return node_ && node_->IsObject(); }

    /**
     * @brief 检查是否为数组
     * @return bool 是数组返回 true，否则返回 false
     */
    bool isArray() const { return node_ && node_->IsArray(); }

    /**
     * @brief 检查是否为数字
     * @return bool 是数字（整数或浮点数）返回 true，否则返回 false
     */
    bool isNumber() const { return node_ && node_->IsNumber(); }

    // ==================== 取值 ====================

    /**
     * @brief 获取节点值（模板方法）
     * @tparam T 目标类型
     * @return T 节点的值
     * @details 如果类型不匹配或节点无效，返回该类型的默认值
     *
     * @warning 类型不匹配时不会抛出异常，而是返回默认值
     * 建议配合 getValueOr() 或 is() 使用以确保类型安全
     */
    template <typename T>
    std::decay_t<T> getValue() const
    {
        if (!isValid() || !is<T>())
        {
            return 0;
        }
        return node_->Get<std::decay_t<T>>();
    }

    /**
     * @brief 获取节点值，带默认值
     * @tparam T 目标类型
     * @param defaultVal 默认值
     * @return T 节点的值，若无效则返回默认值
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("name", "Alice");
     * std::string name = obj.get("name").getValueOr("Unknown");
     * @endcode
     */
    template <typename T>
    std::decay_t<T> getValueOr(T &&defaultVal) const
    {
        if (!isValid() || !node_->Is<T>())
        {
            return defaultVal;
        }
        return node_->Get<std::decay_t<T>>();
    }

    // ==================== 获取子节点 ====================

    /**
     * @brief 获取对象的子节点
     * @param key 键名
     * @return Node 子节点
     * @details 如果键不存在或节点无效，返回错误节点
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("name", "Alice");
     * Node nameNode = obj.get("name");
     * if (nameNode.isValid()) {
     *     std::cout << nameNode.getValue<std::string>() << std::endl;
     * }
     * @endcode
     */
    Node get(const std::string &key) const
    {
        if (!isValid())
        {
            return createError("Cannot get key from invalid node");
        }

        if (!node_->IsObject())
        {
            return createError("Cannot get key from non-object");
        }

        auto it = node_->FindMember(key.c_str());
        if (it == node_->MemberEnd())
        {
            return createError("Key '" + key + "' not found");
        }

        return Node(doc_, &it->value);
    }

    /**
     * @brief 获取数组的元素
     * @param index 索引
     * @return Node 数组元素
     * @details 如果索引越界或节点无效，返回错误节点
     */
    Node at(size_t index) const
    {
        if (!isValid())
        {
            return createError("Cannot access index of invalid node");
        }

        if (!node_->IsArray())
        {
            return createError("Cannot access index of non-array");
        }

        if (index >= node_->Size())
        {
            return createError("Array index out of bounds: " + std::to_string(index));
        }

        return Node(doc_, &(*node_)[index]);
    }

    /**
     * @brief 检查对象是否包含指定键
     * @param key 键名
     * @return bool 包含返回 true，否则返回 false
     */
    bool has(const std::string &key) const
    {
        if (!isValid() || !node_->IsObject())
        {
            return false;
        }
        return node_->HasMember(key.c_str());
    }

    /**
     * @brief 删除对象的键
     * @param key 键名
     * @return Node& 自身引用，支持链式调用
     * @details 如果键不存在，不执行任何操作
     */
    Node &remove(const std::string &key)
    {
        if (!isValid())
        {
            return *this;
        }

        if (!node_->IsObject())
        {
            return *this;
        }

        node_->RemoveMember(key.c_str());
        return *this;
    }

    /**
     * @brief 获取对象或数组的大小
     * @return size_t 大小
     * @details 对象返回键值对数量，数组返回元素数量
     */
    size_t size() const
    {
        if (!isValid())
        {
            return 0;
        }

        if (node_->IsObject())
        {
            return node_->MemberCount();
        }
        if (node_->IsArray())
        {
            return node_->Size();
        }
        return 0;
    }

    /**
     * @brief 获取对象的所有键名
     * @return std::vector<std::string> 键名列表
     * @details 如果节点不是对象，返回空列表
     */
    std::vector<std::string> keys() const
    {
        if (!isValid() || !node_->IsObject())
        {
            return {};
        }

        std::vector<std::string> result;
        for (auto it = node_->MemberBegin(); it != node_->MemberEnd(); ++it)
        {
            result.push_back(it->name.GetString());
        }
        return result;
    }

    // ==================== 设置值 ====================

    /**
     * @brief 设置对象的键值对（模板方法）
     * @tparam T 值类型，支持 std::string, int, int64_t, double, bool 等
     * @param key 键名
     * @param value 值（使用完美转发）
     * @return Node& 自身引用，支持链式调用
     * @details 如果键已存在，覆盖旧值；如果节点无效或非对象节点，不执行任何操作
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("name", "Alice")     // 字符串
     *    .set("age", 30)           // 整数
     *    .set("height", 1.75)      // 浮点数
     *    .set("active", true);     // 布尔值
     * @endcode
     */
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, Node> &&
                              (std::is_constructible_v<std::string, std::decay_t<T>> ||
                               std::is_integral_v<std::decay_t<T>> ||
                               std::is_floating_point_v<std::decay_t<T>>)>>
    Node &set(const std::string &key, T &&value)
    {
        return _setImpl(key, std::forward<T>(value));
    }

    /**
     * @brief 设置对象的键值对 - Node 类型
     * @param key 键名
     * @param value 值节点
     * @return Node& 自身引用，支持链式调用
     * @details 将整个 Node 对象作为值嵌入
     */
    Node &set(const std::string &key, const Node &value)
    {
        if (!isValid() || !node_->IsObject() || !value.isValid())
        {
            return *this;
        }

        // 先移除已存在的键（如果存在）
        node_->RemoveMember(key.c_str());

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.CopyFrom(*value.node_, doc_->GetAllocator());
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    /**
     * @brief 设置子对象
     * @param key 键名
     * @return Node 新创建的子对象
     * @details 如果键已存在且值为对象，重置为空对象；否则创建新的空对象
     *
     * 示例：
     * @code
     * Node config = Node::createObject();
     * Node nested = config.setObject("nested");
     * nested.set("key", "value");
     * @endcode
     */
    Node setObject(const std::string &key)
    {
        if (!isValid() || !node_->IsObject())
        {
            return createError("Cannot set object on invalid or non-object node");
        }

        auto it = node_->FindMember(key.c_str());
        if (it != node_->MemberEnd())
        {
            it->value.SetObject();
            return Node(doc_, &it->value);
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.SetObject();
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        auto newIt = node_->FindMember(key.c_str());
        return Node(doc_, &newIt->value);
    }

    /**
     * @brief 设置子数组
     * @param key 键名
     * @return Node 新创建的子数组
     * @details 如果键已存在且值为数组，重置为空数组；否则创建新的空数组
     *
     * 示例：
     * @code
     * Node config = Node::createObject();
     * Node list = config.setArray("list");
     * list.append(1).append(2).append(3);
     * @endcode
     */
    Node setArray(const std::string &key)
    {
        if (!isValid() || !node_->IsObject())
        {
            return createError("Cannot set array on invalid or non-array node");
        }

        auto it = node_->FindMember(key.c_str());
        if (it != node_->MemberEnd())
        {
            it->value.SetArray();
            return Node(doc_, &it->value);
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.SetArray();
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        auto newIt = node_->FindMember(key.c_str());
        return Node(doc_, &newIt->value);
    }

    // ==================== 数组追加 ====================

    /**
     * @brief 向数组追加元素（模板方法）
     * @tparam T 元素类型，支持 std::string, int, int64_t, double, bool 等
     * @param value 元素值（使用完美转发）
     * @return Node& 自身引用，支持链式调用
     *
     * 示例：
     * @code
     * Node arr = Node::createArray();
     * arr.append("hello")           // 字符串
     *    .append(42)                // 整数
     *    .append(3.14)              // 浮点数
     *    .append(true);             // 布尔值
     * @endcode
     */
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<std::decay_t<T>, Node> &&
                              (std::is_constructible_v<std::string, std::decay_t<T>> ||
                               std::is_integral_v<std::decay_t<T>> ||
                               std::is_floating_point_v<std::decay_t<T>>)>>
    Node &append(T &&value)
    {
        return _appendImpl(std::forward<T>(value));
    }

    /**
     * @brief 向数组追加 Node 元素
     * @param value 节点元素
     * @return Node& 自身引用，支持链式调用
     */
    Node &append(const Node &value)
    {
        if (!isValid() || !node_->IsArray() || !value.isValid())
        {
            return *this;
        }

        rapidjson::Value v;
        v.CopyFrom(*value.node_, doc_->GetAllocator());
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

    /**
     * @brief 追加一个空对象到数组
     * @return Node 新创建的对象
     *
     * 示例：
     * @code
     * Node arr = Node::createArray();
     * Node obj = arr.appendObject();
     * obj.set("name", "Alice");
     * @endcode
     */
    Node appendObject()
    {
        if (!isValid() || !node_->IsArray())
        {
            return createError("Cannot append object to invalid or non-array node");
        }

        rapidjson::Value v;
        v.SetObject();
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return Node(doc_, &(*node_)[node_->Size() - 1]);
    }

    /**
     * @brief 追加一个空数组到数组
     * @return Node 新创建的数组
     *
     * 示例：
     * @code
     * Node arr = Node::createArray();
     * Node subArr = arr.appendArray();
     * subArr.append(1).append(2);
     * @endcode
     */
    Node appendArray()
    {
        if (!isValid() || !node_->IsArray())
        {
            return createError("Cannot append array to invalid or non-array node");
        }

        rapidjson::Value v;
        v.SetArray();
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return Node(doc_, &(*node_)[node_->Size() - 1]);
    }

    // ==================== 序列化 ====================

    /**
     * @brief 将节点序列化为 JSON 字符串
     * @param pretty 是否美化输出（默认 false）
     * @return std::string JSON 字符串
     * @details 无效节点返回 "null"
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("name", "Alice");
     * std::string compact = obj.toJson();       // 紧凑格式
     * std::string formatted = obj.toJson(true); // 美化格式
     * @endcode
     */
    std::string toJson(bool pretty = false) const
    {
        return toJson(pretty, 8);  // 默认精度为 8
    }

    /**
     * @brief 将节点序列化为 JSON 字符串（支持自定义浮点数精度）
     * @param pretty 是否美化格式（缩进和换行）
     * @param decimalPlaces 浮点数精度（1-17），默认值为 8
     * @return std::string JSON 字符串
     * @details 
     * - decimalPlaces 控制浮点数的十进制位数
     * - 取值范围：1-17（RapidJSON 支持的范围）
     * - 如果超出范围，会自动限制在有效范围内（<1 会被限制为 1）
     * - 如果节点无效，返回 "null"
     *
     * 示例：
     * @code
     * Node obj = Node::createObject();
     * obj.set("pi", 3.14159265358979323846);
     * obj.set("e", 2.71828182845904523536);
     *
     * std::string defaultPrecision = obj.toJson();  // 默认 8 位精度
     * std::string lowPrecision = obj.toJson(false, 2);  // 2 位精度
     * std::string highPrecision = obj.toJson(true, 15); // 15 位精度，美化格式
     * @endcode
     */
    std::string toJson(bool pretty, int decimalPlaces) const
    {
        if (!isValid())
        {
            return "null";
        }

        // 限制精度范围在 1-17（RapidJSON 要求最小为 1）
        decimalPlaces = std::max(1, std::min(17, decimalPlaces));

        rapidjson::StringBuffer buffer;

        if (pretty)
        {
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            writer.SetMaxDecimalPlaces(decimalPlaces);
            node_->Accept(writer);
        }
        else
        {
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            writer.SetMaxDecimalPlaces(decimalPlaces);
            node_->Accept(writer);
        }

        return std::string(buffer.GetString(), buffer.GetSize());
    }

    /**
     * @brief 从 JSON 字符串解析节点
     * @param json JSON 字符串
     * @return Node 解析后的节点
     * @details 如果解析失败，返回错误节点，error_ 包含错误信息
     *
     * 示例：
     * @code
     * std::string json = R"({"name": "Alice", "age": 30})";
     * Node obj = Node::fromJson(json);
     * if (obj.isValid()) {
     *     std::cout << obj.get("name").getValue<std::string>() << std::endl;
     * } else {
     *     std::cerr << "Parse error: " << obj.getError() << std::endl;
     * }
     * @endcode
     */
    static Node fromJson(const std::string &json)
    {
        Node result;
        result.doc_ = std::make_shared<rapidjson::Document>();
        result.doc_->Parse(json.c_str());

        if (result.doc_->HasParseError())
        {
            result.error_ = "JSON parse error at offset " +
                            std::to_string(result.doc_->GetErrorOffset());
            result.doc_ = nullptr;
            result.node_ = nullptr;
            return result;
        }

        result.node_ = result.doc_.get();
        return result;
    }
};

} // namespace ConfigParser

// ==================== 模板特化实现 ====================

// getValue<T>() 特化
template <>
inline char ConfigParser::Node::getValue<char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return node_->GetString()[0];
}

template <>
inline signed char ConfigParser::Node::getValue<signed char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return static_cast<signed char>(node_->GetString()[0]);
}

template <>
inline unsigned char ConfigParser::Node::getValue<unsigned char>() const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return '\0';
    }
    return static_cast<unsigned char>(node_->GetString()[0]);
}

template <>
inline std::string ConfigParser::Node::getValue<std::string>() const
{
    if (!isValid() || !node_->IsString())
    {
        return "";
    }
    return std::string(node_->GetString(), node_->GetStringLength());
}

template <>
inline const char *ConfigParser::Node::getValue<const char *>() const
{
    if (!isValid() || !node_->IsString())
    {
        return "";
    }
    return node_->GetString();
}

// getValueOr<T>() 特化
template <>
inline char ConfigParser::Node::getValueOr<char>(char &&defaultVal) const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return defaultVal;
    }
    return node_->GetString()[0];
}

template <>
inline signed char ConfigParser::Node::getValueOr<signed char>(signed char &&defaultVal) const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return defaultVal;
    }
    return static_cast<signed char>(node_->GetString()[0]);
}

template <>
inline unsigned char ConfigParser::Node::getValueOr<unsigned char>(unsigned char &&defaultVal) const
{
    if (!isValid() || !node_->IsString() || node_->GetStringLength() == 0)
    {
        return defaultVal;
    }
    return static_cast<unsigned char>(node_->GetString()[0]);
}

template <>
inline std::string ConfigParser::Node::getValueOr<std::string>(std::string &&defaultVal) const
{
    if (!isValid() || !node_->IsString())
    {
        return defaultVal;
    }
    return std::string(node_->GetString(), node_->GetStringLength());
}

#endif // NODE_H