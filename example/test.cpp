#include "JConfigParser/model/node.h"
#include <string>
#include <iostream>
using namespace std;
using namespace Joger;
using namespace Joger::ConfigParser;

int main(int argc, char const *argv[])
{
    auto root = std::make_shared<MapNode>();
    root->addSubNode("h1", ValNode(std::string("abc")));
    root->addSubNode("h2", MapNode());
    auto h2 = dynamic_pointer_cast<MapNode>((*root)["h2"]);
    h2->addSubNode("h21", ValNode(std::string("123")));
    h2->addSubNode("h22", VecNode());
    auto h22 = dynamic_pointer_cast<VecNode>((*h2)["h22"]);
    h22->emplace_back(ValNode(1));
    h22->emplace_back(ValNode(2));
    h22->emplace_back(ValNode(3));

    h2->addSubNode("h23", VecNode());
    auto h23 = dynamic_pointer_cast<VecNode>((*h2)["h23"]);
    h23->emplace_back(ValNode(std::string("1")));
    h23->emplace_back(ValNode(std::string("2")));
    h23->emplace_back(ValNode(std::string("3")));

    root->addSubNode("h3", VecNode(
                               {MapNode({std::make_pair("h311", ValNode(std::string("123"))),
                                         std::make_pair("h312", ValNode(123))}),
                                MapNode({std::make_pair("h321", ValNode(std::string("123"))),
                                         std::make_pair("h322", ValNode(123.3))})}));

    printf("%s\n", root->toString().c_str());
    return 0;
}
