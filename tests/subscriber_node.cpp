#include <crow/nodes/subscriber_node.h>
#include <crow/tower_cls.h>
#include <crow/gates/loopgate.h>
#include <doctest/doctest.h>

void foo(nos::buffer data)
{
    (void)data;
}

TEST_CASE("doctest")
{
    crow::Tower tower;
    crow::loopgate gate;
    gate.bind(tower, 99);

    crow::subscriber_node node(foo);
    node.bind(tower, 13);
}