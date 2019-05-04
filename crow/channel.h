/**
	@file
*/

#ifndef G2_CORE_H
#define G2_CORE_H

#include <crow/node.h>
#include <igris/buffer.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>

namespace crow
{
	enum class State
	{
		INIT,
		CONNECTED,
		DISCONNECTED,
	};

	enum class Frame
	{
		HANDSHAKE = 0,
		DATA = 1,
		REFUSE = 2,
	};

	struct channel : public crow::node
	{
		dlist_head lnk;
		uint16_t id;
		uint16_t rid;
		void *raddr_ptr;
		size_t raddr_len;
		uint8_t qos;
		uint16_t ackquant;
		uint16_t fid = 0;
		State state = State::INIT;
		void incoming_packet(crow::packet *pack) override final;
		virtual void incoming_data_packet(crow::packet *pack) = 0;
		channel() { dlist_init(&lnk); }

		void handshake(uint8_t *raddr, uint16_t rlen, uint16_t rid,
					   uint8_t qos = 0, uint16_t ackquant = 200);
		void send(const char *data, size_t size);
	};

	struct subheader_channel
	{
		// uint16_t sid;
		// uint16_t rid;
		uint16_t frame_id;
		Frame ftype;
	} G1_PACKED;

	struct subheader_handshake
	{
		uint8_t qos;
		uint16_t ackquant;
	} G1_PACKED;

	static inline subheader_channel *get_subheader_channel(crow::packet *pack)
	{
		return (subheader_channel *)(pack->dataptr() + sizeof(crow::subheader));
	}

	static inline subheader_handshake *
	get_subheader_handshake(crow::packet *pack)
	{
		return (subheader_handshake *)(pack->dataptr() +
									   sizeof(crow::subheader) +
									   sizeof(crow::subheader_channel));
	}

	static inline igris::buffer get_datasect_channel(crow::packet *pack)
	{
		return igris::buffer(pack->dataptr() + sizeof(crow::subheader) +
								 sizeof(crow::subheader_channel),
							 pack->datasize() - sizeof(crow::subheader) -
								 sizeof(crow::subheader_channel));
	}

	// crow::channel* get_channel(uint16_t id);

	// extern igris::dlist<crow::channel, &crow::channel::lnk> channels;

	/// Добавить сервис к ядру.
	void link_channel(crow::channel *srvs, uint16_t id);

	void handshake(crow::channel *ch, uint16_t rid, const void *raddr_ptr,
				   size_t raddr_len, uint8_t qos = 0, uint16_t ackquant = 200);
	void __channel_send(crow::channel *ch, const char *data, size_t size);

	struct accept_header
	{
		uint16_t rchid;
		uint8_t qos;
		uint16_t ackquant;
	};

	struct acceptor : public crow::node
	{
		igris::delegate<crow::channel *> init_channel;
		acceptor(igris::delegate<crow::channel *> init_channel)
			: init_channel(init_channel)
		{
		}

		void incoming_packet(crow::packet *pack) override;
	};

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