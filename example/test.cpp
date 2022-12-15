#include "JConfigParser/model/node.h"
#include <string>
#include <iostream>
using namespace std;
using namespace Joger;
using namespace Joger::ConfigParser;

int main(int argc, char const *argv[])
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
    root->addSubNode("h1", ValNode(std::string("abc")));
    root->addSubNode("h2", MapNode());
    printf("1\n");
    auto h2 = JCP_GET_SUB_NODE_PTR(MapNode, root, "h2");
    printf("1.1\n");
    h2->addSubNode("h21", ValNode(std::string("123")));
    h2->addSubNode("h22", VecNode());
    printf("2.1\n");
    auto h22 = JCP_GET_SUB_NODE_PTR(VecNode, h2, "h22");
    printf("2.2\n");
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
                                         std::make_pair("h312", ValNode(123))}),
                                MapNode({std::make_pair("h321", ValNode(std::string("123"))),
                                         std::make_pair("h322", ValNode(float(123.3)))})}));

    printf("%s\n", root->toString().c_str());
    printf("h322: %s\n", std::to_string(
                             JCP_GET_SUB_NODE_PTR(ValNode, JCP_GET_SUB_NODE_PTR(MapNode, JCP_GET_SUB_NODE_PTR(VecNode, root, "h3"), 1), "h322")->getValue().cast<float>())
                             .c_str());
    return 0;
}
