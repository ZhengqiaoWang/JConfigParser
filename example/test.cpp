#include "jconfig_parser_api.h"
#include <string>
#include <iostream>
using namespace std;
using namespace Joger;
using namespace Joger::ConfigParser;

int main(int argc, char const *argv[])
{
    Any a = 1;
    Any b = string("hello, world");
    Any c;
    std::vector<std::string> td{"123", "hahaha"};
    Any d = td;

    cout << std::boolalpha;
    cout << "a is null:" << a.isNull() << endl;
    cout << "b is null:" << b.isNull() << endl;
    cout << "c is null:" << c.isNull() << endl;

    cout << "----------------------" << endl;
    cout << "a is int:" << a.is<int>() << endl;
    cout << "a is string:" << a.is<string>() << endl;
    cout << "a cast to int:" << a.cast<int>() << endl;
    a.cast<int>() = 123;
    cout << "a cast to int new:" << a.cast<int>() << endl;
    cout << "b cast to string:" << b.cast<string>() << endl;
    for (auto &item : d.cast<std::vector<std::string>>())
    {
        cout << item << endl;
    }
    cout << "----------------------" << endl;
    c = a;
    cout << "c is int:" << c.is<int>() << endl;
    cout << "c is string:" << c.is<string>() << endl;
    cout << "c cast to int:" << c.cast<int>() << endl;
    cout << "c cast to string:" << c.cast<string>() << endl;
    return 0;
}
