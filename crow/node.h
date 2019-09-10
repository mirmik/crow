#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/packet.h>
#include <crow/protocol.h>
#include <sys/uio.h>

#include <igris/container/dlist.h>

#define CROW_NODEPACK_COMMON 0
#define CROW_NODEPACK_ERROR 1

#define CROW_ERRNO_UNREGISTRED_RID 33

namespace crow
{
	struct node_subheader
	{
		uint16_t sid;
		uint16_t rid;
		uint8_t type;
	} __attribute__((packed));

	struct node
	{
		struct dlist_head lnk = DLIST_HEAD_INIT(lnk);
		struct dlist_head waitlnk = DLIST_HEAD_INIT(waitlnk);	
		uint16_t id = 0;
		const char* mnem = NULL;

		virtual void incoming_packet(crow::packet *pack) = 0;
		virtual void undelivered_packet(crow::packet *pack) = 0;
		int waitevent();
		void notify_one(int future);

		virtual const char* typestr() { return "node"; }
	};

	void link_node(node *srvs, uint16_t id);

	class system_node_cls : public node 
	{
		void incoming_packet(crow::packet *pack) override;
		
		void undelivered_packet(crow::packet *pack) override
		{
			crow::release(pack);
		}
	};

	class node_protocol_cls : public crow::protocol 
	{
	private:
		void send_node_error(crow::packet *pack, int errcode);

	public:
		system_node_cls system_node;

	public:
		void incoming(crow::packet *pack) override;
		void undelivered(crow::packet *pack) override;
		
		node_protocol_cls() : protocol(CROW_NODE_PROTOCOL) 
		{
			link_node(&system_node, 0);
			system_node.mnem = "twrinfo";
		}

		static auto sid(crow::packet *pack) { return ((node_subheader*)(pack->dataptr()))->sid; }
		static auto rid(crow::packet *pack) { return ((node_subheader*)(pack->dataptr()))->rid; }
		static auto node_data(crow::packet *pack) 
		{ 
			return igris::buffer( 
			pack->dataptr() + sizeof(node_subheader), 
			pack->datasize() - sizeof(node_subheader)); 
		}
		static auto get_error_code(crow::packet *pack) 
		{ 
			return *(int*)(node_data(pack).data()); 
		}
	};
	extern node_protocol_cls node_protocol;	
	extern igris::dlist<node, &node::lnk> nodes;

	void node_send(uint16_t sid, uint16_t rid, const void *raddr, size_t rlen,
				   const void *data, size_t size, uint8_t qos,
				   uint16_t ackquant);

	/*void node_subheader_init() 
	{

	}*/
}

#endif