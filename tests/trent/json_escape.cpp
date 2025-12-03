#include <doctest/doctest.h>
#include <nos/trent/json.h>
#include <nos/trent/json_print.h>

TEST_CASE("json parser understands escape sequences")
{
    auto tr = nos::json::parse(R"("line\n\t\"quote\"\\\/\b\f\r")");
    std::string expected = "line\n\t\"quote\"\\/\b\f\r";
    CHECK_EQ(tr.as_string(), expected);

    tr = nos::json::parse("'escape\\'single'");
    CHECK_EQ(tr.as_string(), "escape'single");
}

TEST_CASE("json parser decodes unicode escapes and surrogate pairs")
{
    auto smile = nos::json::parse(R"("\u0041\u263A")");
    std::string expected = "A";
    expected += "\xE2\x98\xBA";
    CHECK_EQ(smile.as_string(), expected);

    auto gclef = nos::json::parse(R"("\uD834\uDD1E")");
    std::string clef = "\xF0\x9D\x84\x9E";
    CHECK_EQ(gclef.as_string(), clef);
}

TEST_CASE("json serializer escapes control characters and quotes")
{
    nos::trent tr = std::string("line\n\t\"quote\"\\slash\x01");
    CHECK_EQ(nos::json::to_string(tr),
             "\"line\\n\\t\\\"quote\\\"\\\\slash\\u0001\"");
}

TEST_CASE("json serializer escapes dictionary keys and values")
{
    nos::trent dict;
    dict["line\nkey"] = "value\r";

    auto json = nos::json::to_string(dict);
    CHECK_EQ(json, "{\"line\\nkey\":\"value\\r\"}");
}
