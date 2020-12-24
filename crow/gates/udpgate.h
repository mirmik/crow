#ifndef CROW_GATES_UDPGATE_H
#define CROW_GATES_UDPGATE_H

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

		int open(uint16_t port = 0);
		void finish();

		void bind(int gate_no = CROW_UDPGATE_NO) 
		{
			gateway::bind(gate_no);
		}

#if CROW_ENABLE_WITHOUT_FDS
#else
		int get_fd() { return sock; }
#endif

	};

	udpgate *create_udpgate(uint8_t id);
	udpgate *create_udpgate(uint8_t id, uint16_t port);
}

#endif