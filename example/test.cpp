#include "JConfigParser/style/json/json.h"
#include <string>
#include <iostream>
using namespace std;
using namespace Joger;
using namespace Joger::ConfigParser;

int main(int argc, char const *argv[])
{
    JsonDecoder jc;
    std::string txt = "{ // feqn \n \"h1\":\"a\\\"bc\",\n\"h2\":{ //feqddddw\n    \"h21\":\"123\",\n    \"h22\":[1,\n    2,\n    3], //fee\n    \"h23\":[\"1\"\n    ,\"2\"\n    ,\"3\"]\n},h3:[\n    { // hihihi\n        \"h311\":\"123\",\n        \"h312\":4321 ,\n    },\n    {\n        \"h321\":\"aaa\",\n        \"h322\":234.2\n    }\n]\n}";
    printf("--------> before:\n%s\n", txt.c_str());
    printf("--------> after:\n%s\n", jc.decode(txt)->toString().c_str());
    return 0;
}
