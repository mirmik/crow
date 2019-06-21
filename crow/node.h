#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/packet.h>
#include <crow/protocol.h>
#include <sys/uio.h>

#include <igris/container/dlist.h>

namespace crow
{
	class node_protocol_cls : public crow::protocol 
	{
	public:
		void incoming(crow::packet *pack) override;
		void undelivered(crow::packet *pack) override;
		node_protocol_cls() : protocol(CROW_NODE_PROTOCOL) {}
	};
	extern node_protocol_cls node_protocol;	

	struct node_subheader
	{
		uint16_t sid;
		uint16_t rid;
	} __attribute__((packed));
	
	struct node
	{
		struct dlist_head lnk = DLIST_HEAD_INIT(lnk);
		struct dlist_head waitlnk = DLIST_HEAD_INIT(waitlnk);	
		uint16_t id = 0;

		virtual void incoming_packet(crow::packet *pack) = 0;
		virtual void undelivered_packet(crow::packet *pack) = 0;
		int waitevent();
		void notify_one(int future);
	};

	extern igris::dlist<node, &node::lnk> nodes;

	void link_node(node *srvs, uint16_t id);

	void node_send(uint16_t sid, uint16_t rid, const void *raddr, size_t rlen,
				   const void *data, size_t size, uint8_t qos,
				   uint16_t ackquant);
}

#endif