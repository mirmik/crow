#include <crow/node.h>
#include <crow/nodes/test.h>
#include <crow/nodes/action.h>

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

void incoming(crow::packet* pack) {
	gxx::println("incoming");

	switch(pack->header.f.type) {
		case G1_G0TYPE:
			crow::incoming_node_packet(pack);
			break;
		default:
			crow::release(pack);
	}
}

void hello(crow::packet* pack) {
	gxx::println("HelloWorld!!!!!");
}

int main() {
	crow::create_test_node(10);
	crow::create_action_node(11, hello);
	
	crow::user_incoming_handler = incoming;
	crow::node_send(0, 10, nullptr, 0, "HelloWorld", 10, 0, 200);
	crow::node_send(0, 11, nullptr, 0, "HelloWorld", 10, 0, 200);
	
	crow::spin();
}