#ifndef CROW_PROTO_REQUEST_H
#define CROW_PROTO_REQUEST_H

#include <crow/types.h>
#include <crow/node.h>

namespace crow 
{
	class oneshoot_async_requester : public crow::node
	{
		void (*_dlg)(void*, int, crow::packet *); 
		void * _arg;

	public:
		oneshoot_async_requester(void (*dlg)(void*, int, crow::packet *), void* arg) 
			: _dlg(dlg), _arg(arg)
		{}

	protected:
		void incoming_packet() override 
		{
			_dlg(this, 0, pack);	
			destroy();
		}

		void undelivered_packet() override
		{
			_dlg(this, -1, pack);	
			destroy();
		}

		void destroy() 
		{
			delete this;
		}
	};

	static inline
	oneshoot_async_requester * async_request(
		void (*dlg)(void*, int, crow::packet*), 
		void * arg,
		nid_t rid,
		const crow::hostaddr & addr,
		igris::buffer data,
		uint8_t qos,
		uint16_t ackquant
	) 
	{
		if (qos == 0) 
		{
			dprln("WARN: async_request qos is 0. changed to 2");
			qos = 2;
		}

		oneshoot_async_requester * req = new oneshoot_async_requester(dlg, arg);
		req.bind();

		req.send(rid, addr, data, qos, ackquant);
		return req;
	}

	static inline
	oneshoot_async_requester * async_request_v(
		void (*dlg)(void*, int, crow::packet*), 
		void * arg,
		nid_t rid,
		const crow::hostaddr & addr,
		igris::buffer * data,
		int len,
		uint8_t qos,
		uint16_t ackquant
	) 
	{
		if (qos == 0) 
		{
			dprln("WARN: async_request qos is 0. changed to 2");
			qos = 2;
		}

		oneshoot_async_requester * req = new oneshoot_async_requester(dlg, arg);
		req.bind();

		req.send_(rid, addr, data, len, qos, ackquant);
		return req;
	}
}

#endif