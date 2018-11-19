/**
	@file
*/

#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <crow/gateway.h>

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

	gateway* create_udpgate(uint16_t port, uint8_t id);
}

#endif