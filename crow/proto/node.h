#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/packet.h>
#include <crow/address.h>
#include <crow/proto/protocol.h>

#include <igris/container/dlist.h>

#define CROW_NODEPACK_COMMON 0
#define CROW_NODEPACK_ERROR 1

#define CROW_ERRNO_UNREGISTRED_RID 33

namespace crow
{
	crow::packet_ptr node_send(uint16_t sid,
	                           uint16_t rid,
	                           const crow::hostaddr & addr,
	                           const igris::buffer data,
	                           uint8_t qos,
	                           uint16_t ackquant);

	crow::packet_ptr node_send_v(uint16_t sid,
	                             uint16_t rid,
	                             const crow::hostaddr & addr,
	                             const igris::buffer * vec,
	                             size_t veclen,
	                             uint8_t qos,
	                             uint16_t ackquant);

	struct node_subheader
	{
		uint16_t sid;
		uint16_t rid;
		union
		{
			uint8_t flags = 0;

			struct
			{
				uint8_t namerid : 1;
				uint8_t reserved : 3;
				uint8_t type : 4;
			};
		};
	} __attribute__((packed));

	struct node;
	crow::node * find_node(int id);
	void link_node(node *srvs, uint16_t id);
	void bind_node_dynamic(node *srvs);

	struct node
	{
		struct dlist_head lnk = DLIST_HEAD_INIT(lnk); // Список нодов.
		struct dlist_head waitlnk = DLIST_HEAD_INIT(waitlnk); // Список ожидающих прихода сообщения.
		uint16_t id = 0;
		const char* mnem = NULL;

		virtual void incoming_packet(crow::packet *pack) = 0;
		virtual void undelivered_packet(crow::packet *pack) = 0;
		int waitevent();
		void notify_one(int future);

		virtual const char* typestr()
		{
			return "node";
		}

		node& bind(int addr)
		{
			link_node(this, addr); return *this;
		};

		node& bind()
		{
			bind_node_dynamic(this); return *this;
		};

		crow::packet_ptr send(uint16_t rid,
		                      const crow::hostaddr& raddr,
		                      const igris::buffer data,
		                      uint8_t qos,
		                      uint16_t ackquant)
		{
			return crow::node_send(id, rid, raddr, data, qos, ackquant);
		}

		crow::packet_ptr send_v(uint16_t rid,
		                        const crow::hostaddr& raddr,
		                        const igris::buffer * vdat,
		                        size_t vlen,
		                        uint8_t qos,
		                        uint16_t ackquant)
		{
			return crow::node_send_v(id, rid, raddr, vdat, vlen, qos, ackquant);
		}
	};

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
			crow::node_subheader *sh = (crow::node_subheader *) pack->dataptr();

			if (sh->namerid == 0)
				return igris::buffer(
				           pack->dataptr() + sizeof(node_subheader),
				           pack->datasize() - sizeof(node_subheader));
			else
				return igris::buffer(
				           pack->dataptr() + sizeof(node_subheader) + sh->rid,
				           pack->datasize() - sizeof(node_subheader) - sh->rid);
		}

		static node_subheader* subheader(crow::packet *pack)
		{
			return (crow::node_subheader*) pack->dataptr();
		}

		static auto get_name(crow::packet *pack)
		{
			node_subheader *sh = (crow::node_subheader *) pack->dataptr();

			if (sh->namerid == 0)
				return igris::buffer();
			else
				return igris::buffer(
				           pack->dataptr() + sizeof(node_subheader),
				           sh->rid);
		}

		static auto get_error_code(crow::packet *pack)
		{
			return *(int*)(node_data(pack).data());
		}
	};
	extern node_protocol_cls node_protocol;
	extern igris::dlist<node, &node::lnk> nodes;

	static inline igris::buffer node_data(crow::packet *pack)
	{
		return node_protocol_cls::node_data(pack);
	}



	class node_packet_ptr : public packet_ptr
	{
	public:
		node_packet_ptr(crow::packet *pack_) : packet_ptr(pack_) {}
		node_packet_ptr(const crow::packet_ptr &oth) : packet_ptr(oth) {}
		node_packet_ptr(crow::packet_ptr &&oth) : packet_ptr(std::move(oth)) {}

		igris::buffer message()
		{
			return node_data(pack);
		}
	};
}

#endif