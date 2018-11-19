#include <crow/node.h>
#include <crow/tower.h>
#include <gxx/syslock.h>

gxx::dlist<crow::node, &crow::node::lnk> crow::nodes;

void crow::node_send(uint16_t sid, uint16_t rid, const void* raddr, size_t rsize, const void* data, size_t size, uint8_t qos, uint16_t ackquant)
{
	crow::subheader sh;
	sh.sid = sid;
	sh.rid = rid;

	struct iovec iov[2] =
	{
		{ (void*)&sh, sizeof(sh) },
		{ (void*)data, size }
	};

	crow::send_v(raddr, rsize, iov, 2, G1_G0TYPE, qos, ackquant);
}

void crow::incoming_node_handler(crow::packet* pack)
{
	crow::subheader* sh = crow::get_subheader(pack);

	for (crow::node& srvs : crow::nodes)
	{
		if (srvs.id == sh->rid)
		{
			srvs.incoming_packet(pack);
			return;
		}
	}

	crow::release(pack);
}

void crow::link_node(crow::node* srv, uint16_t id)
{
	srv->id = id;
	nodes.add_last(*srv);
}