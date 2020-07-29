#ifndef CROW_PROTO_RETRANSLER_H
#define CROW_PROTO_RETRANSLER_H

#include <igris/event/delegate.h>
#include <crow/proto/node.h>

namespace crow 
{
	class retransler : public crow::node
	{
		crow::hostaddr host_a;
		crow::hostaddr host_b;

		uint16_t aid;
		uint16_t bid;

		igris::delegate<void, retransler *, crow::packet *> error_handler;

	protected:
		void init(uint16_t aid, crow::hostaddr host_a, uint16_t bid, crow::hostaddr host_b) 
		{
			this->aid = aid;
			this->bid = bid;
			this->host_a = host_a;
			this->host_b = host_b;

			bind();
		}

		void incoming_packet(crow::packet * pack) 
		{
			auto sh = crow::node::subheader(pack);

			if (aid == sh->sid && host_a == pack->addr()) 
			{
				send(bid, host_b, crow::node_data(pack), 
					pack->header.qos, pack->header.ackquant);
			}

			if (bid == sh->sid && host_b == pack->addr()) 
			{
				send(aid, host_a, crow::node_data(pack), 
					pack->header.qos, pack->header.ackquant);
			}
		}

		void undelivered_packet(crow::packet * pack) 
		{
			error_handler(this, pack);
			crow::release(pack);		
		}
	};
}

#endif