#include <crow/node.h>
#include <crow/nodes/test.h>
#include <crow/nodes/action.h>
#include <crow/tower.h>
#include <crow/indexes.h>
#include <gxx/print.h>
#include <gxx/syslock.h>

gxx::dlist<crow::node, &crow::node::lnk> crow::nodes;

void crow::__node_send(uint16_t sid, uint16_t rid, const void* raddr, size_t rsize, const void* data, size_t size, crow::QoS qos, uint16_t ackquant) {		
	crow::subheader sh;
	sh.sid = sid;
	sh.rid = rid;

	gxx::iovec iov[2] = {
		{ &sh, sizeof(sh) },
		{ data, size }
	};

	crow::send(raddr, rsize, iov, 2, G1_G0TYPE, qos, ackquant);
}


void crow::incoming_node_packet(crow::packet* pack) {
	auto sh = get_subheader(pack);
	for ( auto& srvs: crow::nodes ) {
		if (srvs.id == sh->rid) {
			srvs.incoming_packet(pack);
			return;
		}
	}
	gxx::fprintln("crow: unresolved node {}. release packet", (uint16_t)sh->rid);
	crow::release(pack);	
}

void crow::link_node(crow::node* srv, uint16_t id) {
	srv->id = id;
	crow::nodes.move_back(*srv);
}

crow::test_node* crow::create_test_node(int port) {
	auto tsrv = new crow::test_node();
	crow::link_node(tsrv, port);
	return tsrv;
}

crow::action_node* crow::create_action_node(int port, gxx::delegate<void, crow::packet*> dlg) {
	auto asrv = new crow::action_node(dlg);
	crow::link_node(asrv, port);
	return asrv;
}