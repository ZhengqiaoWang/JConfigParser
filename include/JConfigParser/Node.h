#ifndef NODE_H
#define NODE_H

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

class Node
{
private:
    std::shared_ptr<rapidjson::Document> doc_;
    rapidjson::Value *node_;
    std::string error_;

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

    // 创建单个值的Document（用于构造函数）- 使用完美转发
    template <typename T>
    static void _createValueDocument(Node &node, T &&value)
    {
        node.doc_ = std::make_shared<rapidjson::Document>();

        if constexpr (std::is_constructible_v<std::string, std::decay_t<T>> ||
                      std::is_same_v<std::decay_t<T>, const char*>)
        {
            std::string str(std::forward<T>(value));
            node.doc_->SetString(str.c_str(), node.doc_->GetAllocator());
        }
        else
        {
            // 其他类型（int, int64_t, double, bool等）直接使用Set
            node.doc_->Set(std::forward<T>(value));
        }

        node.node_ = node.doc_.get();
    }

    // 创建rapidjson::Value（用于set和append）- 使用完美转发
    template <typename T>
    rapidjson::Value _createValue(T &&value) const
    {
        rapidjson::Value v;

        if constexpr (std::is_constructible_v<std::string, std::decay_t<T>> ||
                      std::is_same_v<std::decay_t<T>, const char*>)
        {
            std::string str(std::forward<T>(value));
            v.SetString(str.c_str(), doc_->GetAllocator());
        }
        else
        {
            // 其他类型（int, int64_t, double, bool等）直接使用Set
            v.Set(std::forward<T>(value));
        }

        return v;
    }

    // 通用的set方法实现 - 使用完美转发
    template <typename T>
    Node &_setImpl(const std::string &key, T &&value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v = _createValue(std::forward<T>(value));
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    // 通用的append方法实现 - 使用完美转发
    template <typename T>
    Node &_appendImpl(T &&value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v = _createValue(std::forward<T>(value));
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

public:
    // ==================== 构造 ====================

    // 默认构造: null 节点
    Node() : doc_(nullptr), node_(nullptr) {}

    // 创建空对象
    static Node createObject()
    {
        Node obj;
        obj.doc_ = std::make_shared<rapidjson::Document>();
        obj.doc_->SetObject();
        obj.node_ = obj.doc_.get();
        return obj;
    }

    // 创建空数组
    static Node createArray()
    {
        Node arr;
        arr.doc_ = std::make_shared<rapidjson::Document>();
        arr.doc_->SetArray();
        arr.node_ = arr.doc_.get();
        return arr;
    }

    // 值构造 - 支持多种类型，使用完美转发
    template <typename T, typename = std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, Node> &&
        (std::is_constructible_v<std::string, std::decay_t<T>> ||
         std::is_integral_v<std::decay_t<T>> ||
         std::is_floating_point_v<std::decay_t<T>>)>>
    Node(T &&val)
    {
        _createValueDocument(*this, std::forward<T>(val));
    }

    // Node类型的拷贝构造（避免与模板冲突）
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

    // Node类型的移动构造
    Node(Node &&other) noexcept
        : doc_(std::move(other.doc_)), node_(other.node_), error_(std::move(other.error_))
    {
        other.node_ = nullptr;
    }

    // ==================== 拷贝与克隆 ====================

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

    Node clone() const
    {
        if (!doc_)
        {
            return Node();
        }

        Node cloned;
        cloned.doc_ = std::make_shared<rapidjson::Document>();
        cloned.doc_->CopyFrom(*doc_, cloned.doc_->GetAllocator());

        if (node_ == doc_.get())
        {
            cloned.node_ = cloned.doc_.get();
        }
        else
        {
            cloned.node_ = cloned.doc_.get();
        }

        return cloned;
    }

    // ==================== 状态判断 ====================

    bool isValid() const
    {
        return doc_ != nullptr && node_ != nullptr;
    }

    bool isNull() const
    {
        return node_ && node_->IsNull();
    }

    bool isError() const
    {
        return !error_.empty();
    }

    std::string getError() const
    {
        return error_;
    }

    // ==================== 类型判断 ====================

    template <typename T>
    bool is() const;

    bool isString() const { return node_ && node_->IsString(); }
    bool isInt64() const { return node_ && node_->IsInt64(); }
    bool isDouble() const { return node_ && node_->IsDouble(); }
    bool isBool() const { return node_ && node_->IsBool(); }
    bool isObject() const { return node_ && node_->IsObject(); }
    bool isArray() const { return node_ && node_->IsArray(); }
    bool isNumber() const { return node_ && node_->IsNumber(); }

    // ==================== 取值 ====================

    template <typename T>
    T getValue() const;

    template <typename T>
    T getValueOr(const T &defaultVal) const;

    // ==================== 获取子节点 ====================

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

    bool has(const std::string &key) const
    {
        if (!isValid() || !node_->IsObject())
        {
            return false;
        }
        return node_->HasMember(key.c_str());
    }

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

    // 模板set方法 - 支持各种类型，使用完美转发
    template <typename T, typename = std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, Node> &&
        (std::is_constructible_v<std::string, std::decay_t<T>> ||
         std::is_integral_v<std::decay_t<T>> ||
         std::is_floating_point_v<std::decay_t<T>>)>>
    Node &set(const std::string &key, T &&value)
    {
        return _setImpl(key, std::forward<T>(value));
    }

    // Node类型特殊处理 - 使用完美转发
    Node &set(const std::string &key, const Node &value)
    {
        if (!isValid() || !node_->IsObject() || !value.isValid())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.CopyFrom(*value.node_, doc_->GetAllocator());
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    // 直接创建子对象/子数组
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

    // 模板append方法 - 支持各种类型，使用完美转发
    template <typename T, typename = std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, Node> &&
        (std::is_constructible_v<std::string, std::decay_t<T>> ||
         std::is_integral_v<std::decay_t<T>> ||
         std::is_floating_point_v<std::decay_t<T>>)>>
    Node &append(T &&value)
    {
        return _appendImpl(std::forward<T>(value));
    }

    // Node类型特殊处理
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

    std::string toJson(bool pretty = false) const
    {
        if (!isValid())
        {
            return "null";
        }

        rapidjson::StringBuffer buffer;

        if (pretty)
        {
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            node_->Accept(writer);
        }
        else
        {
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            node_->Accept(writer);
        }

        return std::string(buffer.GetString(), buffer.GetSize());
    }

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

// ==================== 模板特化实现 ====================

// is<T>() 特化
template <>
inline bool Node::is<std::string>() const
{
    return node_ && node_->IsString();
}

template <>
inline bool Node::is<const char*>() const
{
    return node_ && node_->IsString();
}

template <>
inline bool Node::is<int64_t>() const
{
    return node_ && node_->IsInt64();
}

template <>
inline bool Node::is<double>() const
{
    return node_ && node_->IsDouble();
}

template <>
inline bool Node::is<bool>() const
{
    return node_ && node_->IsBool();
}

// getValue<T>() 特化
template <>
inline std::string Node::getValue<std::string>() const
{
    if (!isValid() || !node_->IsString())
    {
        return "";
    }
    return std::string(node_->GetString(), node_->GetStringLength());
}

template <>
inline const char* Node::getValue<const char*>() const
{
    if (!isValid() || !node_->IsString())
    {
        return "";
    }
    return node_->GetString();
}

template <>
inline int64_t Node::getValue<int64_t>() const
{
    if (!isValid() || !node_->IsInt64())
    {
        return 0;
    }
    return node_->GetInt64();
}

template <>
inline double Node::getValue<double>() const
{
    if (!isValid() || !node_->IsDouble())
    {
        return 0.0;
    }
    return node_->GetDouble();
}

template <>
inline bool Node::getValue<bool>() const
{
    if (!isValid() || !node_->IsBool())
    {
        return false;
    }
    return node_->GetBool();
}

// getValueOr<T>() 特化
template <>
inline std::string Node::getValueOr<std::string>(const std::string &defaultVal) const
{
    if (!isValid() || !node_->IsString())
    {
        return defaultVal;
    }
    return std::string(node_->GetString(), node_->GetStringLength());
}

template <>
inline int64_t Node::getValueOr<int64_t>(const int64_t &defaultVal) const
{
    if (!isValid() || !node_->IsInt64())
    {
        return defaultVal;
    }
    return node_->GetInt64();
}

template <>
inline double Node::getValueOr<double>(const double &defaultVal) const
{
    if (!isValid() || !node_->IsDouble())
    {
        return defaultVal;
    }
    return node_->GetDouble();
}

template <>
inline bool Node::getValueOr<bool>(const bool &defaultVal) const
{
    if (!isValid() || !node_->IsBool())
    {
        return defaultVal;
    }
    return node_->GetBool();
}

#endif // NODE_H