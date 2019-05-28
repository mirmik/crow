/**
	@file
*/

#ifndef G2_CORE_H
#define G2_CORE_H

#include <crow/node.h>

#include <igris/buffer.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>

#define CROW_CHANNEL_INIT 0
#define CROW_CHANNEL_CONNECTED 1
#define CROW_CHANNEL_DISCONNECTED 2

namespace crow
{
	enum class State : uint8_t
	{
		INIT = 0,
		CONNECTED = 1,
		DISCONNECTED = 2,
	};

	enum class Frame : uint8_t
	{
		HANDSHAKE = 0,
		DATA = 1,
		REFUSE = 2,
	};

	class channel;
	void link_channel(crow::channel *srvs, uint16_t id);

	class channel : public crow::node
	{
	public:
		using incoming_handler_t = void(*)(crow::channel*, crow::packet*);

		dlist_head lnk;
		uint16_t rid;
		void *raddr_ptr;
		size_t raddr_len;
		uint8_t qos;
		uint16_t ackquant;
		uint16_t fid = 0;
		State _state = State::INIT;
		incoming_handler_t incoming_handler;

		channel() : lnk(DLIST_HEAD_INIT(lnk)) {};
		channel(incoming_handler_t incoming_handler) : lnk(DLIST_HEAD_INIT(lnk)),
			incoming_handler(incoming_handler) {}

		void init(int id, incoming_handler_t incoming_handler)
		{
			dprln("init");
			DPRINT(id);
			this->incoming_handler = incoming_handler;
			crow::link_channel(this, id);
		}

		uint8_t state() { return (uint8_t)_state; }

		void incoming_packet(crow::packet *pack) override;
		void incoming_data_packet(crow::packet *pack);

		void undelivered_packet(crow::packet *pack) override;

		void handshake(const uint8_t *raddr, uint16_t rlen, uint16_t rid,
		               uint8_t qos = 0, uint16_t ackquant = 200);

		void send(const char *data, size_t size);

		static igris::buffer getdata(crow::packet *pack);
	};

	struct acceptor : public crow::node
	{
		igris::delegate<crow::channel *> init_channel;

		acceptor() = default;
		acceptor(igris::delegate<crow::channel *> init_channel)
			: init_channel(init_channel)
		{}

		void init(int id, igris::delegate<crow::channel *> init_channel)
		{
			this->init_channel = init_channel;
			link_node(this, id);
		}

		void incoming_packet(crow::packet *pack) override;
		void undelivered_packet(crow::packet *pack) override;
	};

	struct subheader_channel
	{
		uint16_t frame_id;
		Frame ftype;
	} __attribute__((packed));

	struct subheader_handshake
	{
		uint8_t qos;
		uint16_t ackquant;
	} __attribute__((packed));

	/*struct accept_header
	{
		uint16_t rchid;
		uint8_t qos;
		uint16_t ackquant;
	} __attribute__((packed));*/


	static inline subheader_channel *get_subheader_channel(crow::packet *pack)
	{
		return (subheader_channel *)(pack->dataptr() + sizeof(crow::node_subheader));
	}

	static inline subheader_handshake *
	get_subheader_handshake(crow::packet *pack)
	{
		return (subheader_handshake *)(pack->dataptr() +
		                               sizeof(crow::node_subheader) +
		                               sizeof(crow::subheader_channel));
	}

	/*	static inline igris::buffer get_datasect_channel(crow::packet *pack)
		{
			return igris::buffer(pack->dataptr() + sizeof(crow::node_subheader) +
									 sizeof(crow::subheader_channel),
								 pack->datasize() - sizeof(crow::node_subheader) -
									 sizeof(crow::subheader_channel));
		}
	*/
	// crow::channel* get_channel(uint16_t id);

	// extern igris::dlist<crow::channel, &crow::channel::lnk> channels;

	/// Добавить сервис к ядру.

	void __channel_send(crow::channel *ch, const char *data, size_t size);



	static inline acceptor *
	create_acceptor(uint16_t port, igris::delegate<crow::channel *> dlg)
	{
		auto asrv = new crow::acceptor(dlg);
		crow::link_node(asrv, port);
		return asrv;
	}

	uint16_t dynport();
}; // namespace crow

#endif