#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "JConfigParser/model/node.h"

using namespace Joger::Model;
using namespace Joger::ConfigParser;

TEST_CASE("test Node basic type")
{
    /*
    效果如下所示
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
    */
    auto root = std::make_shared<MapNode>();
    {
        root->addSubNode("h1", ValNode(std::string("abc")));
        root->addSubNode("h2", MapNode());
        auto h2 = JCP_GET_SUB_NODE_PTR(MapNode, root, "h2");
        h2->addSubNode("h21", ValNode(std::string("123")));
        h2->addSubNode("h22", VecNode());
        auto h22 = JCP_GET_SUB_NODE_PTR(VecNode, h2, "h22");
        h22->emplace_back(ValNode(1));
        h22->emplace_back(ValNode(2));
        h22->emplace_back(ValNode(3));

        h2->addSubNode("h23", VecNode());
        auto h23 = JCP_GET_SUB_NODE_PTR(VecNode, h2, "h23");
        h23->emplace_back(ValNode(std::string("1")));
        h23->emplace_back(ValNode(std::string("2")));
        h23->emplace_back(ValNode(std::string("3")));

        root->addSubNode("h3", VecNode(
                                   {MapNode({std::make_pair("h311", ValNode(std::string("123"))),
                                             std::make_pair("h312", ValNode(4321))}),
                                    MapNode({std::make_pair("h321", ValNode(std::string("aaa"))),
                                             std::make_pair("h322", ValNode(double(234.2)))})}));
    }

    // 开始检查
    {
        CHECK(JCP_GET_SUB_NODE_PTR(ValNode, root, "h1")->getValue().cast<std::string>() == "abc");

        auto h2 = JCP_GET_SUB_NODE_PTR(MapNode, root, "h2");
        CHECK(JCP_GET_SUB_NODE_PTR(ValNode, h2, "h21")->getValue().cast<std::string>() == "123");

        int i = 1;
        for (auto &item : *JCP_GET_SUB_NODE_PTR(VecNode, h2, "h22"))
        {
            CHECK(JCP_GET_CUR_NODE_PTR(ValNode, item)->getValue().cast<int>() == i);
            ++i;
        }

        auto h3 = JCP_GET_SUB_NODE_PTR(VecNode, root, "h3");
        for (size_t j = 0; j < (*h3).size(); ++j)
        {
            auto &item = (*h3)[j];
            if (j == 0)
            {
                auto map_item = JCP_GET_CUR_NODE_PTR(MapNode, item);
                CHECK(JCP_GET_SUB_NODE_PTR(ValNode, map_item, "h311")->getValue().cast<std::string>() == "123");
                CHECK(JCP_GET_SUB_NODE_PTR(ValNode, map_item, "h312")->getValue().cast<int>() == 4321);
            }
            else if (j == 1)
            {
                auto map_item = JCP_GET_CUR_NODE_PTR(MapNode, item);
                CHECK(JCP_GET_SUB_NODE_PTR(ValNode, map_item, "h321")->getValue().cast<std::string>() == "aaa");
                CHECK(JCP_GET_SUB_NODE_PTR(ValNode, map_item, "h322")->getValue().cast<double>() == 234.2);
            }
        }
    }
}
