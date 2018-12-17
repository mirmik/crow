/**
	@file
*/

#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <crow/gateway.h>
#include <crow/defs.h>

namespace crow 
{

	struct udpgate : public gateway
	{
		int sock;
		crow::packet* block;

		void send(crow::packet*) override;
		void nblock_onestep() override;

		int open(uint16_t  port);
	};

	udpgate* create_udpgate(uint8_t id, uint16_t port);
}

#endif