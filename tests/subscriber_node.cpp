#include <crow/nodes/subscriber_node.h>
#include <doctest/doctest.h>

void foo(igris::buffer data, crow::subscriber_node&) 
{
	(void) data;
}

TEST_CASE("doctest") 
{
	crow::subscriber_node node(foo);

	node.bind(13);
}