#ifndef CROW_SOCKET_H
#define CROW_SOCKET_H

#include <crow/proto/node.h>

namespace crow 
{
	class socket_base : public node 
	{
		void incoming_packet(crow::packet *pack) 
		{
			dlist_add(&pack->ulnk, &q);
		}
	}

	class socket : public socket_base
	{
		dlist_head q = DLIST_INIT(q);
		bool is_alive = true;

		void *raddr_ptr = nullptr;
		size_t raddr_len = 0;

	public:
		socket() {}

		void undelivered_packet(crow::packet *pack) 
		{
			is_alive = false;
			crow::release(pack);
		}


	};

	class socket_acceptor_basic : public socket_base
	{
		dlist_head q = DLIST_INIT(q);

	public:
		socket() {}

		virtual socket* socket_create() = 0;

		void incoming_packet(crow::packet *pack) 
		{
			dlist_add(&pack->ulnk, &q);
		}
	};

	class socket_acceptor : public socket_acceptor_basic 
	{
	public:
		virtual socket* socket_create(crow::packet * request) 
		{
			auto addr = request.addr();
			socket* ret = new socket();
			ret->raddr_ptr = new uint8_t[addr.size()];
			memcpy(ret->raddr_ptr, addr.data(), addr.size());
			return ret;
		}
	};
}

#endif