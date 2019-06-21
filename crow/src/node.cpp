#include <crow/node.h>
#include <crow/defs.h>
#include <crow/tower.h>
#include <igris/sync/syslock.h>

igris::dlist<crow::node, &crow::node::lnk> crow::nodes;
crow::node_protocol_cls crow::node_protocol;	

void crow::node_send(uint16_t sid, uint16_t rid, const void *raddr,
					 size_t rsize, const void *data, size_t size, uint8_t qos,
					 uint16_t ackquant)
{
	crow::node_subheader sh;
	sh.sid = sid;
	sh.rid = rid;

	struct iovec iov[2] = {{(void *)&sh, sizeof(sh)}, {(void *)data, size}};

	crow::send_v(raddr, rsize, iov, 2, CROW_NODE_PROTOCOL, qos, ackquant);
}

void crow::node_protocol_cls::incoming(crow::packet *pack)
{
	crow::node_subheader *sh = (crow::node_subheader *) pack->dataptr();

	for (crow::node &srvs : crow::nodes)
	{
		if (srvs.id == sh->rid)
		{
			srvs.incoming_packet(pack);
			return;
		}
	}

	crow::release(pack);
}

void crow::node_protocol_cls::undelivered(crow::packet *pack) 
{
	crow::node_subheader *sh = (crow::node_subheader *) pack->dataptr();

	for (crow::node &srvs : crow::nodes)
	{
		if (srvs.id == sh->sid)
		{
			srvs.undelivered_packet(pack);
			return;
		}
	}

	crow::release(pack);	
}

void crow::link_node(crow::node *srv, uint16_t id)
{
	srv->id = id;
	system_lock();
	nodes.add_last(*srv);
	system_unlock();
}