#include <doctest/doctest.h>
#include <nos/trent/json.h>

TEST_CASE("json parses numeric scalars with whitespace and comments")
{
    nos::trent tr;

    tr = nos::json::parse("13");
    CHECK_EQ(tr.as_numer(), 13);

    tr = nos::json::parse("0.5");
    CHECK_EQ(tr.as_numer(), 0.5);

    tr = nos::json::parse("-0.5");
    CHECK_EQ(tr.as_numer(), -0.5);

    tr = nos::json::parse("/*13*/ 42");
    CHECK_EQ(tr.as_numer(), 42);

    tr = nos::json::parse(" 20e-1 ");
    CHECK_EQ(tr.as_numer(), doctest::Approx(2.0));
}

TEST_CASE("json parses arrays and dicts through comments and newlines")
{
    auto dict = nos::json::parse("{'a':42, /*aaa*/ 'b' : 13}");
    CHECK(dict.is_dict());

    dict = nos::json::parse("{'a':42, /*aaa*/ 'b' : 13 }");
    CHECK(dict.is_dict());

    auto list = nos::json::parse(R"(
        [42, //aaa
        13]
    )");
    CHECK(list.is_list());

    list = nos::json::parse(R"(

        [42,

    /*aaa*/

    13]

    )");
    CHECK(list.is_list());
}

TEST_CASE("json parses booleans and nil/null mnemonics")
{
    auto tr = nos::json::parse(" {'a': false} ");
    CHECK(tr["a"].is_bool());
    CHECK_FALSE(tr["a"].as_bool());

    tr = nos::json::parse(" {'a': false } ");
    CHECK(tr["a"].is_bool());
    CHECK_FALSE(tr["a"].as_bool());

    tr = nos::json::parse("nil");
    CHECK(tr.is_nil());

    tr = nos::json::parse("{'a': nil}");
    CHECK(tr["a"].is_nil());

    tr = nos::json::parse("null");
    CHECK(tr.is_nil());

    tr = nos::json::parse("{'a': null}");
    CHECK(tr["a"].is_nil());
}
