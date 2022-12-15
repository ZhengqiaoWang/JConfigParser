#ifndef JCONFIG_PARSER_NODE_H
#define JCONFIG_PARSER_NODE_H

#include "JConfigParser/model/any.h"
#include <vector>
#include <map>
#include <memory>
#include <iostream>

namespace Joger
{
    namespace ConfigParser
    {
        enum class NodeType
        {
            VAL,
            VEC,
            MAP
        };
        class ValNode;
        class VecNode;
        class MapNode;
        class NodeBase
        {
        public:
            virtual ~NodeBase() {}

        public:
            NodeType getNodeType() { return m_node_type; }
            virtual std::string toString() = 0;

        protected:
            NodeType m_node_type{NodeType::VAL};
        };

        class ValNode : public NodeBase
        {
            using ValNodeValType = Model::Any;

        public:
            ValNode()
            {
                m_node_type = NodeType::VAL;
            }
            ValNode(const ValNode &src)
            {
                m_node_type = NodeType::VAL;
                m_val = src.m_val;
            }
            ValNode(const ValNodeValType &src)
            {
                m_node_type = NodeType::VAL;
                m_val = std::move(src);
            }

        public:
            void setValue(const ValNodeValType &val)
            {
                m_val = val;
            }

            ValNodeValType &getValue() { return m_val; }

        public:
            virtual std::string toString() override;

        private:
            ValNodeValType m_val;
        };

        class VecNode : public NodeBase
        {
            using VecNodeItemType = std::shared_ptr<NodeBase>;
            using VecNodeValType = std::vector<VecNodeItemType>;
            using VecNodeValIterType = VecNodeValType::iterator;

        public:
            VecNode()
            {
                m_node_type = NodeType::VEC;
            }
            VecNode(const VecNode &src)
            {
                m_node_type = NodeType::VEC;
                m_val_vec = src.m_val_vec;
            }
            VecNode(const VecNodeValType &src)
            {
                m_node_type = NodeType::VEC;
                m_val_vec = std::move(src);
            }
            template <typename T>
            VecNode(const std::initializer_list<T> &src)
            {
                m_node_type = NodeType::VEC;
                for (auto &item : src)
                {
                    emplace_back(std::move(item));
                }
            }

        public:
            template <typename SubNodeType>
            void emplace_back(const SubNodeType &val) { m_val_vec.emplace_back(std::shared_ptr<NodeBase>(dynamic_cast<NodeBase *>(new SubNodeType(val)))); }
            VecNodeItemType &at(size_t idx) { return m_val_vec[idx]; }
            VecNodeItemType &operator[](size_t n) { return at(n); }
            VecNodeValIterType del(const VecNodeValIterType &iter) { return m_val_vec.erase(iter); }

            VecNodeValIterType begin() { return m_val_vec.begin(); }
            VecNodeValIterType end() { return m_val_vec.end(); }

            virtual std::string toString() override;

            size_t size() { return m_val_vec.size(); }

        private:
            VecNodeValType m_val_vec;
        };

        /* =========================== MapNode ===========================*/
        class MapNode : public NodeBase
        {
            using MapNodeKeyType = std::string;
            using MapNodeItemType = std::shared_ptr<NodeBase>;
            using MapNodeValType = std::map<std::string, MapNodeItemType>;
            using MapNodeValIterType = MapNodeValType::iterator;

        public:
            MapNode()
            {
                m_node_type = NodeType::MAP;
            }
            MapNode(const MapNode &src)
            {
                m_node_type = NodeType::MAP;
                m_node_ptr_map = src.m_node_ptr_map;
            }
            MapNode(const MapNodeValType &src)
            {
                m_node_type = NodeType::MAP;
                m_node_ptr_map = std::move(src);
            }
            template <typename T>
            MapNode(const std::initializer_list<T> &src)
            {
                m_node_type = NodeType::VEC;
                for (auto &item : src)
                {
                    addSubNode(std::move(item.first), std::move(item.second));
                }
            }

        public:
            template <typename SubNodeType>
            void addSubNode(const MapNodeKeyType &key, const SubNodeType &val)
            {
                m_node_ptr_map.emplace(std::make_pair(key, std::shared_ptr<NodeBase>(dynamic_cast<NodeBase *>(new SubNodeType(val)))));
            }
            void delSubNode(const MapNodeKeyType &key)
            {
                m_node_ptr_map.erase(key);
            }
            MapNodeItemType &operator[](const MapNodeKeyType &key) { return m_node_ptr_map[key]; }

            MapNodeValIterType find(const MapNodeKeyType &key) { return m_node_ptr_map.find(key); }
            MapNodeValIterType begin() { return m_node_ptr_map.begin(); }
            MapNodeValIterType end() { return m_node_ptr_map.end(); }
            virtual std::string toString() override;

            size_t size() { return m_node_ptr_map.size(); }

        private:
            MapNodeValType m_node_ptr_map;
        };

        /* =========================== Tools ===========================*/
/**
 * @brief Get Val Node PTR
 * @param type sub node type: NodeType
 * @param root root node shared_ptr
 *
 */
#define JCP_GET_CUR_NODE_PTR(type, item_ptr) std::dynamic_pointer_cast<type>((item_ptr))

/**
 * @brief Get Sub Node
 * @param type sub node type: NodeType
 * @param root root node shared_ptr
 * @param key sub node's string key
 *
 */
#define JCP_GET_SUB_NODE_PTR(type, root, key) std::dynamic_pointer_cast<type>((*root)[key])



        /* =========================== ValNode ===========================*/
        inline std::string ValNode::toString()
        {

#define _MACRO_NODE_TOSTRING_CAST(TYPE)            \
    else if (m_val.is<TYPE>())                     \
    {                                              \
        return std::to_string(m_val.cast<TYPE>()); \
    }

            if (m_val.isNull())
            {
                return "(null)";
            }
            else if (m_val.is<std::string>())
            {
                return std::string("\"") + m_val.cast<std::string>() + "\"";
            }
            else if (m_val.is<char *>())
            {
                return std::string("\"") + m_val.cast<char *>() + "\"";
            }
            _MACRO_NODE_TOSTRING_CAST(float)
            _MACRO_NODE_TOSTRING_CAST(double)
            _MACRO_NODE_TOSTRING_CAST(int)
            _MACRO_NODE_TOSTRING_CAST(unsigned int)
            _MACRO_NODE_TOSTRING_CAST(long)
            _MACRO_NODE_TOSTRING_CAST(unsigned long)
            _MACRO_NODE_TOSTRING_CAST(short)
            _MACRO_NODE_TOSTRING_CAST(unsigned short)
            printf("failed to toString\n");
            throw std::bad_cast();
            return "(ERROR)";
        }

        /* =========================== VecNode ===========================*/
        inline std::string VecNode::toString()
        {
            std::string result{"["};
            for (size_t i = 0; i < m_val_vec.size(); ++i)
            {
                auto ptr = m_val_vec[i];
                switch (ptr->getNodeType())
                {
                case NodeType::VAL:
                {
                    result += dynamic_cast<ValNode *>(ptr.get())->toString();
                    break;
                }
                case NodeType::VEC:
                {
                    result += dynamic_cast<VecNode *>(ptr.get())->toString();
                    break;
                }
                case NodeType::MAP:
                {
                    result += dynamic_cast<MapNode *>(ptr.get())->toString();
                    break;
                }
                default:
                {
                    break;
                }
                }
                if (i < m_val_vec.size() - 1)
                {
                    result += ",";
                }
            }
            return result + "]";
        }

        /* =========================== MapNode ===========================*/
        inline std::string MapNode::toString()
        {
            std::string result{"{"};
            for (auto iter = m_node_ptr_map.begin(); iter != m_node_ptr_map.end(); ++iter)
            {
                auto &key = iter->first;
                auto &ptr = iter->second;
                result += "\"" + key + "\":";
                switch (ptr->getNodeType())
                {
                case NodeType::VAL:
                {
                    result += dynamic_cast<ValNode *>(ptr.get())->toString();
                    break;
                }
                case NodeType::VEC:
                {
                    result += dynamic_cast<VecNode *>(ptr.get())->toString();
                    break;
                }
                case NodeType::MAP:
                {
                    result += dynamic_cast<MapNode *>(ptr.get())->toString();
                    break;
                }
                default:
                {
                    break;
                }
                }
                auto next_iter = iter;
                next_iter++;
                if (next_iter != (m_node_ptr_map.end()))
                {
                    result += ",";
                }
            }
            return result + "}";
        }
    }
}

#endif