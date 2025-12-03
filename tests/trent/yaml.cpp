#include <doctest/doctest.h>
#include <nos/trent/yaml.h>

TEST_CASE("yaml parses simple mapping with scalars")
{
    auto tr = nos::yaml::parse(R"(name: demo
count: 3
flag: true
empty: ~
)");

    CHECK_EQ(tr["name"].as_string(), "demo");
    CHECK_EQ(tr["count"].as_numer(), 3);
    CHECK(tr["flag"].as_bool());
    CHECK(tr["empty"].is_nil());
}

TEST_CASE("yaml parses sequences and nested mappings")
{
    auto tr = nos::yaml::parse(R"(items:
  - name: one
    value: 1
  - name: two
    value: 2
)");

    const auto &list = tr["items"].as_list();
    REQUIRE_EQ(list.size(), 2);
    CHECK_EQ(list[0]["name"].as_string(), "one");
    CHECK_EQ(list[0]["value"].as_numer(), 1);
    CHECK_EQ(list[1]["name"].as_string(), "two");
    CHECK_EQ(list[1]["value"].as_numer(), 2);
}

TEST_CASE("yaml printer emits minimal representation")
{
    nos::trent doc;
    doc["name"] = "demo config";
    doc["enabled"] = true;
    doc["values"].as_list().push_back(1);
    doc["values"].as_list().push_back(2);
    doc["values"].as_list().push_back(3);

    auto yaml = nos::yaml::to_string(doc);
    CHECK_EQ(yaml,
             "name: \"demo config\"\n"
             "enabled: true\n"
             "values:\n"
             "  - 1\n"
             "  - 2\n"
             "  - 3\n");
}