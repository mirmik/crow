#include <doctest/doctest.h>
#include <nos/trent/json.h>
#include <nos/trent/json_print.h>

TEST_CASE("json serializes numeric nodes")
{
    nos::trent tr = 13;
    CHECK_EQ(nos::json::to_string(tr), "13");
}

TEST_CASE("json serializer keeps dictionary insertion order")
{
    nos::trent tr;
    std::string json;

    tr["a"] = 1;
    tr["c"] = 3;
    tr["b"] = 2;
    json = nos::json::to_string(tr);
    CHECK_EQ(json, R"({"a":1,"c":3,"b":2})");

    tr["a"] = 1;
    tr["b"] = 3;
    tr["c"] = 2;
    json = nos::json::to_string(tr);
    CHECK_EQ(json, R"({"a":1,"c":2,"b":3})");
}

TEST_CASE("json pretty printer formats dictionaries with indentation")
{
    nos::trent tr;
    tr["a"] = 1;
    tr["b"] = 2;
    std::string pretty = nos::json::to_string(tr, true);
    CHECK_EQ(pretty, "{\n\t\"a\": 1,\n\t\"b\": 2\n}\r\n");
}

TEST_CASE("json pretty printer formats lists with indentation")
{
    nos::trent tr;
    tr.as_list().push_back(1);
    tr.as_list().push_back(2);
    std::string pretty = nos::json::to_string(tr, true);
    CHECK_EQ(pretty, "[\r\n\t1,\r\n\t2\r\n]\r\n");
}
