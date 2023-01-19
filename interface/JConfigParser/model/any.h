#ifndef JCONFIG_PARSER_ANY_H
#define JCONFIG_PARSER_ANY_H
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <exception>
namespace Joger
{
    namespace Model
    {
        /**
         * @brief 转换异常类，用于处理从Any转换为各类问题时的异常
         * 自定义该类的原因是std::bad_cast提示信息不完整。
         * 有该类的原因是dynamic_cast<ptr>不会提示std::bad_cast
         *
         */
        class BadCast : public std::exception
        {
        public:
            BadCast(const std::string &src_type, const std::string &tgt_type, const std::string &err_msg) : m_src_type(src_type), m_target_type(tgt_type), m_usr_err_msg(err_msg)
            {
                m_err_msg = "Cannot cast from type[" + m_src_type + "] to type[" + m_target_type + "]. " + m_usr_err_msg;
            }

            virtual const char *what() const noexcept override
            {

                return m_err_msg.c_str();
            }

        private:
            std::string m_src_type{"{unknowntype}"};
            std::string m_target_type{"{unknowntype}"};
            std::string m_usr_err_msg{""};
            std::string m_err_msg{""};
        };

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
            bool is() const noexcept
            {
                return m_tpIndex == std::type_index(typeid(U));
            }

            // 将 Any 转换为实际的类型
            template <class U>
            U &cast()
            {
                if (!is<U>())
                {
                    throw BadCast(m_tpIndex.name(), typeid(U).name(), "unexpect cast");
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
    }
}

#endif