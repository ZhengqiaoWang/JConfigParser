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

    // 字符串构造
    Node(const std::string &val)
    {
        doc_ = std::make_shared<rapidjson::Document>();
        doc_->SetString(val.c_str(), doc_->GetAllocator());
        node_ = doc_.get();
    }

    // const char* 构造
    Node(const char *val)
    {
        doc_ = std::make_shared<rapidjson::Document>();
        doc_->SetString(val, doc_->GetAllocator());
        node_ = doc_.get();
    }

    // int64_t 构造
    Node(int64_t val)
    {
        doc_ = std::make_shared<rapidjson::Document>();
        doc_->SetInt64(val);
        node_ = doc_.get();
    }

    // double 构造
    Node(double val)
    {
        doc_ = std::make_shared<rapidjson::Document>();
        doc_->SetDouble(val);
        node_ = doc_.get();
    }

    // bool 构造
    Node(bool val)
    {
        doc_ = std::make_shared<rapidjson::Document>();
        doc_->SetBool(val);
        node_ = doc_.get();
    }

    // ==================== 拷贝与克隆 ====================

    Node(const Node &other)
        : doc_(other.doc_), node_(other.node_), error_(other.error_)
    {
        // 如果是根节点，深拷贝整个 Document
        if (other.doc_ && other.node_ == other.doc_.get())
        {
            doc_ = std::make_shared<rapidjson::Document>();
            doc_->CopyFrom(*other.doc_, doc_->GetAllocator());
            node_ = doc_.get();
        }
    }

    Node &operator=(const Node &other)
    {
        if (this != &other)
        {
            doc_ = other.doc_;
            node_ = other.node_;
            error_ = other.error_;

            // 如果是根节点，深拷贝
            if (other.doc_ && other.node_ == other.doc_.get())
            {
                doc_ = std::make_shared<rapidjson::Document>();
                doc_->CopyFrom(*other.doc_, doc_->GetAllocator());
                node_ = doc_.get();
            }
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
            // 对于非根节点，需要重新定位到正确的位置
            // 这里简化处理，只支持根节点的克隆
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

    Node &set(const std::string &key, const std::string &value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v(value.c_str(), doc_->GetAllocator());
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &set(const std::string &key, const char *value)
    {
        return set(key, std::string(value));
    }

    Node &set(const std::string &key, int64_t value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.SetInt64(value);
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &set(const std::string &key, double value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.SetDouble(value);
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &set(const std::string &key, bool value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        rapidjson::Value k(key.c_str(), doc_->GetAllocator());
        rapidjson::Value v;
        v.SetBool(value);
        node_->AddMember(k.Move(), v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &set(const std::string &key, const Node &value)
    {
        if (!isValid() || !node_->IsObject())
        {
            return *this;
        }

        if (!value.isValid())
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
            return createError("Cannot set array on invalid or non-object node");
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

    Node &append(const std::string &value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v(value.c_str(), doc_->GetAllocator());
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &append(const char *value)
    {
        return append(std::string(value));
    }

    Node &append(int value)
    {
        return append(static_cast<int64_t>(value));
    }

    Node &append(int64_t value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v;
        v.SetInt64(value);
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &append(double value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v;
        v.SetDouble(value);
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &append(bool value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        rapidjson::Value v;
        v.SetBool(value);
        node_->PushBack(v.Move(), doc_->GetAllocator());

        return *this;
    }

    Node &append(const Node &value)
    {
        if (!isValid() || !node_->IsArray())
        {
            return *this;
        }

        if (!value.isValid())
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