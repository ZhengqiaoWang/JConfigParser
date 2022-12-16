#ifndef JCONFIG_STYLE_JSON_H
#define JCONFIG_STYLE_JSON_H

#include "JConfigParser/model/node.h"
#include <deque>
#include <functional>
#include <assert.h>
#include <unordered_map>

namespace Joger
{
    namespace ConfigParser
    {
        class JsonDecoder
        {
        public:
            enum ValueType : int
            {
                SPLITER,
                KEY,
                VALUE
            };

            std::shared_ptr<MapNode> decode(const std::string &txt)
            {
                clear();
                if (false == analyseString(txt))
                {
                    return nullptr;
                }

                return makeNodes();
            }

        private:
            void clear()
            {
                m_container_deque.clear();
            }

            bool analyseString(const std::string &txt)
            {
                size_t key_start_pos{0}, key_end_pos{0}, last_sign_pos{0};
                for (size_t pos = 0; pos < txt.size(); ++pos)
                {
                    auto &word = txt[pos];

                    if (isQuotation(txt, pos, word))
                    {
                        m_quot_mark_flag = !m_quot_mark_flag;
                        continue;
                    }

                    if (m_quot_mark_flag)
                    {
                        continue;
                    }

                    if (isStartSpliter(word))
                    {
                        m_analyse_deque.push_back(std::make_pair(ValueType::SPLITER, std::string("") + word));
                        key_start_pos = pos;
                        last_sign_pos = pos;
                        continue;
                    }

                    if (':' == word)
                    {
                        m_analyse_deque.push_back(std::make_pair(ValueType::KEY, txt.substr(key_start_pos + 1, pos - key_start_pos - 1)));
                        key_end_pos = pos;
                        last_sign_pos = pos;
                        continue;
                    }

                    if (',' == word || isEndSpliter(word))
                    {
                        std::string val_str = txt.substr(std::max(key_end_pos, last_sign_pos) + 1, pos - std::max(key_end_pos, last_sign_pos) - 1);

                        if (inValidValue(val_str))
                        {
                            m_analyse_deque.push_back(std::make_pair(ValueType::VALUE, val_str));
                        }
                        m_analyse_deque.push_back(std::make_pair(ValueType::SPLITER, std::string("") + word));
                        key_start_pos = pos;
                        key_end_pos = key_start_pos;
                        last_sign_pos = pos;
                        continue;
                    }
                }
                if (m_quot_mark_flag)
                {
                    return false;
                }
                return true;
            }

            std::unordered_map<std::string, std::function<void(size_t)>> doWhenSignal{
                std::make_pair(",", [&](size_t i)
                               { doWhenSignalComma(i); }),
                std::make_pair("{", [&](size_t i)
                               { doWhenSignalLeftBracket(i); }),
                std::make_pair("}", [&](size_t i)
                               { doWhenSignalRightBracket(i); }),
                std::make_pair("[", [&](size_t i)
                               { doWhenSignalLeftSquare(i); }),
                std::make_pair("]", [&](size_t i)
                               { doWhenSignalRightSquare(i); }),
            };

            void doWhenSignalComma(size_t i) {}
            void doWhenSignalLeftBracket(size_t i)
            {
                auto cur_node = m_container_deque.back();
                switch (cur_node->getNodeType())
                {
                case NodeType::MAP:
                {
                    auto cur_key = m_analyse_deque[i - 1].second;
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(MapNode, cur_node);
                    auto new_node = cur_node_with_type->addSubNode(cur_key, MapNode());
                    m_container_deque.push_back(new_node);
                    break;
                }
                case NodeType::VEC:
                {
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(VecNode, cur_node);
                    auto new_node = cur_node_with_type->emplace_back(MapNode());
                    m_container_deque.push_back(new_node);
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
            void doWhenSignalRightBracket(size_t i)
            {
                m_container_deque.pop_back();
            }

            void doWhenSignalLeftSquare(size_t i)
            {
                auto cur_node = m_container_deque.back();
                switch (cur_node->getNodeType())
                {
                case NodeType::MAP:
                {
                    auto cur_key = m_analyse_deque[i - 1].second;
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(MapNode, cur_node);
                    auto new_node = cur_node_with_type->addSubNode(cur_key, VecNode());
                    m_container_deque.push_back(new_node);
                    break;
                }
                case NodeType::VEC:
                {
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(VecNode, cur_node);
                    auto new_node = cur_node_with_type->emplace_back(VecNode());
                    m_container_deque.push_back(new_node);
                    break;
                }
                default:
                {
                    break;
                }
                }
            }

            void doWhenSignalRightSquare(size_t i)
            {
                doWhenSignalRightBracket(i);
            }

            void doWhenItemType(size_t i, const std::string item_val)
            {
                auto cur_node = m_container_deque.back();
                switch (cur_node->getNodeType())
                {
                case NodeType::MAP:
                {
                    auto cur_key = m_analyse_deque[i - 1].second;
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(MapNode, cur_node);
                    cur_node_with_type->addSubNode(cur_key, autoFormatVal(item_val));
                    break;
                }
                case NodeType::VEC:
                {
                    auto cur_node_with_type = JCP_GET_CUR_NODE_PTR(VecNode, cur_node);
                    cur_node_with_type->emplace_back(autoFormatVal(item_val));
                    break;
                }
                default:
                {
                    break;
                }
                }
            }

            std::shared_ptr<MapNode> makeNodes()
            {
                if (m_analyse_deque.empty() || m_analyse_deque.size() <= 2)
                {
                    return nullptr;
                }
                auto root = std::make_shared<MapNode>();
                m_container_deque.push_back(root);

                for (size_t i = 1; i < m_analyse_deque.size(); ++i)
                {
                    auto &item = m_analyse_deque[i];
                    auto &item_type = item.first;
                    auto &item_val = item.second;

                    freshString(item_val, item_type == ValueType::KEY);

                    if (item_type == ValueType::SPLITER)
                    {
                        std::shared_ptr<NodeBase> next_node_ptr;
                        doWhenSignal[item_val](i);
                        continue;
                    }
                    else if (item_type == ValueType::VALUE)
                    {
                        doWhenItemType(i, item_val);
                        continue;
                    }
                }

                return root;
            }

            bool freshString(std::string &txt, bool drop_quotation = false)
            {
                size_t substr_start{0}, substr_end{txt.size() - 1};
                // 清除前部注释与空行
                bool quot_mark_flag{false};
                for (size_t i = 0; i < txt.size(); ++i)
                {
                    auto &word = txt[i];
                    if (isQuotation(txt, i, word))
                    {
                        substr_start = i;
                    }
                    else if (' ' == word || '\n' == word)
                    {
                        continue;
                    }
                    else if ('/' == word)
                    {
                        if (i != (txt.size() - 1) && '/' == txt[i + 1])
                        {
                            bool need_continue{false};
                            for (size_t j = i + 2; j < txt.size(); ++j)
                            {
                                if ('\n' == txt[j])
                                {
                                    i = j;
                                    need_continue = true;
                                    break;
                                }
                            }
                            if (need_continue)
                            {
                                continue;
                            }
                        }
                    }
                    substr_start = i;
                    substr_end = substr_start;
                    break;
                }

                assert(quot_mark_flag == false);

                // 清除后部注释与空行
                for (size_t i = substr_end; i < txt.size(); ++i)
                {
                    auto &word = txt[i];
                    if (isQuotation(txt, i, word))
                    {
                        quot_mark_flag = !quot_mark_flag;
                    }
                    if (quot_mark_flag)
                    {
                        continue;
                    }
                    if (' ' == word || '\n' == word)
                    {
                        substr_end = i - 1;
                        break;
                    }
                    else if ('/' == word)
                    {
                        if (i != txt.size() - 1 && '/' == txt[i + 1])
                        {
                            substr_end = i - 1;
                            break;
                        }
                    }
                    substr_end = i;
                }

                if (drop_quotation && txt[substr_start] == '"' && txt[substr_end] == '"')
                {
                    substr_start++;
                    substr_end--;
                }
                txt = txt.substr(substr_start, substr_end - substr_start + 1);
                return true;
            }

            ValNode autoFormatVal(const std::string &txt)
            {
                if (txt[0] == '"' && txt[txt.size() - 1] == '"')
                {
                    // string
                    return ValNode(txt.substr(1, txt.size() - 2));
                }
                // 判断整数与小数，整数统一用long，小数统一double
                bool ps_flag{false}, dot_flag{false};
                for (size_t i = 0; i < txt.size(); ++i)
                {
                    if (txt[i] < '0' || txt[i] > '9')
                    {
                        if (txt[i] == '-' || txt[i] == '+')
                        {
                            if (ps_flag == true)
                            {
                                // 有一个正负号了，直接认为是字符串吧
                                return ValNode(txt);
                            }
                            ps_flag = true;
                        }
                        if (txt[i] == '.')
                        {
                            if (dot_flag == true)
                            {
                                // 有一个.了，直接认为是字符串吧
                                return ValNode(txt);
                            }
                            dot_flag = true;
                        }
                    }
                }
                if (dot_flag)
                {
                    return ValNode(atof(txt.c_str()));
                }
                else
                {
                    return ValNode(atol(txt.c_str()));
                }
            }

            bool isQuotation(const std::string &txt, const size_t &pos, const char &word)
            {
                if ('"' == word && (pos == 0 || txt[pos - 1] != '\\'))
                {
                    return true;
                }
                return false;
            }

            bool isStartSpliter(const char word)
            {
                if ('{' == word || '[' == word)
                {
                    return true;
                }
                return false;
            }

            bool isEndSpliter(const char word)
            {
                if ('}' == word || ']' == word)
                {
                    return true;
                }
                return false;
            }

            bool inValidValue(const std::string &txt)
            {
                if (txt.size() == 0)
                {
                    return false;
                }
                for (auto &item : txt)
                {
                    if ('\n' != item && ' ' != item)
                    {
                        return true;
                    }
                }
                return false;
            }

        private:
            bool m_quot_mark_flag{false};
            std::deque<std::pair<ValueType, std::string>> m_analyse_deque;
            std::deque<std::shared_ptr<NodeBase>> m_container_deque;
        };
    }
}

#endif