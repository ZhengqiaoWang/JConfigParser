#ifndef JCONFIG_STYLE_JSON_ENCODER_H
#define JCONFIG_STYLE_JSON_ENCODER_H
#include "JConfigParser/model/node.h"
#include <deque>
#include <functional>
#include <assert.h>
#include <unordered_map>

namespace Joger
{
    namespace ConfigParser
    {
        class JsonEncoder
        {
        public:
            std::string encode(const std::shared_ptr<MapNode> &root)
            {
                return root->toString();
            }
        };
    }
}

#endif