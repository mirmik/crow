#include <crow/node.h>
#include <crow/nodes/test.h>
#include <crow/nodes/action.h>

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

void hello(crow::packet* pack) {
	gxx::println("HelloWorld!!!!!");
}

int main() {
	crow::create_test_node(10);
	crow::create_action_node(11, hello);
	
	crow::node_handler = crow::incoming_node_handler;
	crow::node_send(0, 10, nullptr, 0, "HelloWorld", 10, 0, 200);
	crow::node_send(0, 11, nullptr, 0, "HelloWorld", 10, 0, 200);
	
	crow::spin();
}