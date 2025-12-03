#include <cmath>
#include <doctest/doctest.h>
#include <nos/trent/json.h>

TEST_CASE("trent stores nested numerics through operator indexing")
{
    nos::trent tr;

    tr["a"]["b"][28] = 42;
    CHECK_EQ(tr["a"]["b"][28].as_numer(), 42);

    tr[3]["name"] = 19;
    CHECK_EQ(tr[3]["name"].as_integer(), 19);
}

TEST_CASE("trent path resolves both numeric and string steps")
{
    nos::trent tr;
    tr["a"]["b"][28] = 42;
    CHECK_EQ(tr[nos::trent_path("a/b/28")].as_numer(), 42);

    tr[28]["a"]["b"] = 13;
    CHECK_EQ(tr[nos::trent_path("28/a/b")].as_numer(), 13);

    tr[7] = 77;
    CHECK_EQ(tr[nos::trent_path("7")].as_numer(), 77);
}

TEST_CASE("trent stores booleans and preserves them after json parsing")
{
    nos::trent tr;
    tr["a"][28] = true;
    CHECK(tr[nos::trent_path("a/28")].as_bool());

    tr["a"][29] = false;
    CHECK_FALSE(tr[nos::trent_path("a/29")].as_bool());

    auto parsed = nos::json::parse(" {'a': false } ");
    CHECK(parsed["a"].is_bool());
    CHECK_FALSE(parsed["a"].as_bool());
}

TEST_CASE("trent get helpers return pointers and throw with readable messages")
{
    nos::trent tr;

    tr["a"]["b"] = 3;
    CHECK_EQ(tr.get("a/b"), &tr["a"]["b"]);

    tr[7]["a"]["b"] = 3;
    CHECK_EQ(tr.get("7/a/b"), &tr[7]["a"]["b"]);

    tr["a"]["b"] = 3;
    CHECK_EQ(tr.get_as_numer_ex("a/b"), 3);

    tr["A"][7] = 8;
    CHECK_EQ(tr.get_as_numer_ex("A/7"), 8);

    tr[7] = 8;
    CHECK_EQ(tr.get_as_numer_ex("7"), 8);

    tr[7]["A"] = 8;
    CHECK_EQ(tr.get_as_numer_ex("7/A"), 8);

    tr["a"]["b"][28] = -123.513;
    CHECK(std::fabs(tr.get_as_numer_ex("a/b/28") + 123.513) < 1e-5);

    CHECK_THROWS_WITH(tr.get_as_numer_ex("a/c/28"), "trent:wrong_path: a/c/28");

    tr["a"]["b"][28] = "hello";
    CHECK_THROWS_WITH(tr.get_as_numer_ex("a/b/28"),
                      "trent:wrong_type: path:a/b/28 request:num realtype:str");
}

TEST_CASE("trent templated get works for const and mutable references")
{
    nos::trent tr = 3;
    CHECK_EQ(tr.get<int>(), 3);
    CHECK_EQ(std::stod(tr.get<std::string>()), 3);

    const nos::trent &ctr = tr;
    CHECK_EQ(ctr.get<int>(), 3);
    CHECK_EQ(std::stod(ctr.get<std::string>()), 3);
}

TEST_CASE("trent const access and nil defaults")
{
    nos::trent tr;
    tr["a"] = 3;
    const nos::trent &ctr = tr;

    CHECK_EQ(ctr["a"].as_numer_default(42), 3);
    const nos::trent &missing = ctr["c"];
    CHECK(missing.is_nil());
    CHECK_EQ(missing.as_numer_default(42), 42);
    CHECK_EQ(&missing, &nos::trent::static_nil());

    tr["value"] = nos::trent::nil();
    CHECK(tr["value"].is_nil());
    CHECK_EQ(tr["value"].as_numer_default(5), 5);

    tr["value"] = 7;
    CHECK_EQ(tr["value"].as_numer_default(5), 7);
}
