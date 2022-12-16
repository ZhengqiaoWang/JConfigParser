#ifndef JCONFIG_PARSER_API_H
#define JCONFIG_PARSER_API_H
#include <iostream>
#include "JConfigParser/model/node.h"

namespace Joger
{
    namespace ConfigParser
    {
        class JConfigParser
        {
        public:
            template <typename Decoder>
            std::shared_ptr<MapNode> decode(const std::string &txt)
            {
                Decoder decoder;
                return decoder.decode(txt);
            }

            template <typename Encoder>
            std::string encode(const std::shared_ptr<MapNode> nodes)
            {
                Encoder encoder;
                return encoder.encode(nodes);
            }
        };
    }
}

#endif