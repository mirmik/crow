/**
	@file
*/

#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <crow/defs.h>
#include <crow/gateway.h>
#include <iostream>

namespace crow
{

	class udpgate : public gateway
	{
		int sock = 0;
		crow::packet *block = nullptr;

	public:
		udpgate(){}
		udpgate(uint16_t port) { open(port); }

		void send(crow::packet *) override;
		void nblock_onestep() override;

		int open(uint16_t port);
		void finish();

#if CROW_ENABLE_WITHOUT_FDS
#else
		int get_fd() { return sock; }
#endif

	};

	udpgate *create_udpgate(uint8_t id, uint16_t port);
} // namespace crow

#endif