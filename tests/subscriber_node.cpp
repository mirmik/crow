#include <crow/nodes/subscriber_node.h>
#include <doctest/doctest.h>

void foo(igris::buffer data) 
{
	(void) data;
}

TEST_CASE("doctest") 
{
	crow::subscriber_node node(foo);

	node.bind(13);
}