#include "JConfigParser/model/node.h"
#include <string>
#include <iostream>
using namespace std;
using namespace Joger;
using namespace Joger::ConfigParser;

int main(int argc, char const *argv[])
{
    /*
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

    MapNode *root = new MapNode();
    root->addSubNode("h1", ValNode(std::string("abc")));
    root->addSubNode("h2", MapNode());
    auto h2 = dynamic_cast<MapNode*>((*root)["h2"]);
    h2->addSubNode("h21",ValNode(std::string("123")));
    h2->addSubNode("h22",VecNode());
    auto h22 = dynamic_cast<VecNode*>((*h2)["h22"]);
    h22->emplace_back(ValNode(1));
    h22->emplace_back(ValNode(2));
    h22->emplace_back(ValNode(3));

    h2->addSubNode("h23",VecNode());
    auto h23 = dynamic_cast<VecNode*>((*h2)["h23"]);
    h23->emplace_back(ValNode(std::string("1")));
    h23->emplace_back(ValNode(std::string("2")));
    h23->emplace_back(ValNode(std::string("3")));

    root->addSubNode("h3", VecNode(
        {
            MapNode({
                std::make_pair("h311", ValNode(std::string("123"))),
                std::make_pair("h312", ValNode(123))
            }),
            MapNode({
                std::make_pair("h321", ValNode(std::string("123"))),
                std::make_pair("h322", ValNode(123.3))
            })
        }
    ));
    
    printf("%s\n", root->toString().c_str());
    return 0;
}
