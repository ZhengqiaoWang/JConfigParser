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

        class NodeBase
        {
        public:
            enum class NodeType
            {
                VAL,
                VEC,
                DICT
            };

            NodeType getNodeType() { return m_node_type; }
            size_t getSubNodeCnt() { return m_subnode_vec.size(); }

        protected:
            NodeType m_node_type{NodeType::VAL};
            std::vector<NodeBase *> m_subnode_vec;
        };

        class ValNode : public NodeBase
        {
        };
    }

}

#endif