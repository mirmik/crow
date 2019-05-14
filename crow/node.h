/**
	@file
*/

#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/packet.h>
#include <sys/uio.h>

#include <igris/container/dlist.h>

namespace crow
{
	extern void (*node_handler)(crow::packet *pack);

	struct node
	{
		struct dlist_head lnk;
		uint16_t id;
		virtual void incoming_packet(crow::packet *pack) = 0;
	};

	extern igris::dlist<node, &node::lnk> nodes;

	struct subheader
	{
		uint16_t sid;
		uint16_t rid;
	} __attribute__((packed));

	static inline subheader *get_subheader(crow::packet *pack)
	{
		return (subheader *)pack->dataptr();
	}

	void link_node(node *srvs, uint16_t id);

	void node_send(uint16_t sid, uint16_t rid, const void *raddr, size_t rlen,
				   const void *data, size_t size, uint8_t qos,
				   uint16_t ackquant);

	void incoming_node_handler(crow::packet *pack);

	static inline void enable_node_subsystem()
	{
		crow::node_handler = crow::incoming_node_handler;
	}
} // namespace crow

#endif