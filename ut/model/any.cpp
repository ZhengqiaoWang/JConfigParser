#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "JConfigParser/model/any.h"

using namespace Joger::Model;

TEST_CASE("test Any basic type")
{
    Any a;
    CHECK(a.isNull());
    a = 123;
    CHECK(a.isNull() == false);
    CHECK(a.is<int>());
    a = 123.4;
    CHECK(a.is<double>());

    try
    {
        a.cast<int>();
    }
    catch(const BadCast& e)
    {
        std::cout << e.what() << '\n';
    }

    CHECK_THROWS_AS(a.cast<int>(), BadCast);

}

TEST_CASE("test Any c11 types")
{
    Any a(std::string("abc"));
    CHECK(a.is<std::string>());

    Any b(a);
    CHECK(b.is<std::string>());

    Any c = std::vector<int>{1, 2, 3};
    CHECK(c.is<std::vector<int>>());
    auto &c1 = c.cast<std::vector<int>>();
    c1.emplace_back(4);

    int i = 1;
    for (auto &item : c.cast<std::vector<int>>())
    {
        CHECK(item == i);
        ++i;
    }
}