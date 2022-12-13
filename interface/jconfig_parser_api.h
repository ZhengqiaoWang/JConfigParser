#ifndef JCONFIG_PARSER_API_H
#define JCONFIG_PARSER_API_H
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <exception>

namespace Joger
{
    namespace ConfigParser
    {
        /**
         * @brief 参考https://www.cnblogs.com/qicosmos/p/3420095.html实现Any类
         * 
         */
        class Any
        {
        public:
            Any() : m_tpIndex(std::type_index(typeid(void))) {}
            Any(const Any &that) : m_ptr(that.clone()), m_tpIndex(that.m_tpIndex) {}
            Any(Any &&that) : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {}

            // 创建智能指针时，对于一般的类型，通过 std::decay 来移除引用和 cv 符（即 const/volatile），从而获取原始类型
            template <typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type>
            Any(U &&value) : m_ptr(new Derived<typename std::decay<U>::type>(std::forward<U>(value))),
                             m_tpIndex(std::type_index(typeid(typename std::decay<U>::type))) {}

            bool isNull() const { return !bool(m_ptr); }

            /// @note 类型不相同
            template <class U>
            bool is() const
            {
                return m_tpIndex == std::type_index(typeid(U));
            }

            // 将 Any 转换为实际的类型
            template <class U>
            U &cast()
            {
                if (!is<U>())
                {
                    printf("cannot cast [%s] to [%s]\n", m_tpIndex.name(), typeid(U).name());
                    throw std::bad_cast();
                }

                /// @note 将基类指针转为实际的派生类型
                auto derived = dynamic_cast<Derived<U> *>(m_ptr.get());
                return derived->m_value; ///< 获取原始数据
            }

            Any &operator=(const Any &a)
            {
                if (m_ptr == a.m_ptr)
                    return *this;

                m_ptr = a.clone();
                m_tpIndex = a.m_tpIndex;
                return *this;
            }

        private:
            struct Base;
            typedef std::unique_ptr<Base> BasePtr;

            struct Base
            {
                virtual ~Base() {}
                virtual BasePtr clone() const = 0;
            };

            template <typename T>
            struct Derived : Base
            {
                template <typename U>
                Derived(U &&value) : m_value(std::forward<U>(value)) {}

                /// @note 将派生类对象赋值给了基类指针，通过基类擦除了派生类的原始数据类型
                BasePtr clone() const
                {
                    return BasePtr(new Derived<T>(m_value)); ///< 用 unique_ptr 指针进行管理
                }

                T m_value;
            };

            BasePtr clone() const
            {
                if (m_ptr != nullptr)
                {
                    return m_ptr->clone();
                }

                return nullptr;
            }

        private:
            BasePtr m_ptr;
            std::type_index m_tpIndex; ///< 保存的类型
        };

        class ValNode;
        class VecNode;
        class MapNode;
        class NodeBase
        {
        public:
            enum class NodeType
            {
                VAL,
                VEC,
                MAP
            };

            NodeType getNodeType() { return m_node_type; }
            // size_t getSubNodeCnt() { return m_subnode_vec.size(); }
            

        protected:
            NodeType m_node_type{NodeType::VAL};
            // std::vector<NodeBase *> m_subnode_vec;
        };

        class ValNode : public NodeBase
        {
        public:
            ValNode()
            {
                m_node_type=NodeType::VAL;
            }

        public:
            void setValue(const Any &val)
            {
                m_val = val;
            }

            Any &getValue() { return m_val; }

        private:
            Any m_val;
        };

        class VecNode : public NodeBase
        {
        public:
            VecNode()
            {
                m_node_type = NodeType::VEC;
            }

        public:
            void add(Any &&val) { m_val_vec.emplace_back(std::move(val)); }
            Any &at(size_t idx) { return m_val_vec[idx]; }
            Any &operator[](size_t n) { return at(n); }
            std::vector<Any>::iterator del(const std::vector<Any>::iterator &iter) { return m_val_vec.erase(iter); }

            std::vector<Any>::iterator begin() { return m_val_vec.begin(); }
            std::vector<Any>::iterator end() { return m_val_vec.end(); }
        private:
            std::vector<Any> m_val_vec;
        };

        class MapNode : public NodeBase
        {
        public:
            MapNode()
            {
                m_node_type = NodeType::MAP;
            }
        };
    }

    /*
     *
     {
        "h1":"abc",
        "h2":{
            "h21":"123",
            "h22":[1,2,3],
            "h23":["1","2","3"]
        },
        "h3":[
            {
                "h311":"123",
                "h312":4321
            },
            {
                "h321":"aaa",
                "h322":234.2
            }
        ]
     } 
     map{
        value,
        map{
            value,
            vec(value),
            vec(value)
        },
        vec(
            map{
                value,
                value
            },
            map{
                value,
                value
            }
        )
     }
     * 
     */

}

#endif