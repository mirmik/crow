#include <crow/node.h>
#include <crow/nodes/action.h>
#include <crow/nodes/test.h>

#include <crow/gates/udpgate.h>
#include <crow/tower.h>

void hello(crow::packet *pack) { owl::println("HelloWorld!!!!!"); }

int main() {
	crow::create_test_node(10);
	crow::create_action_node(11, hello);

	crow::enable_node_subsystem();
	crow::node_send(0, 10, nullptr, 0, "HelloWorld", 10, 0, 200);
	crow::node_send(0, 11, nullptr, 0, "HelloWorld", 10, 0, 200);

	crow::spin();
}