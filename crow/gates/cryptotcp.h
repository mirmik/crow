#ifndef CROW_GATES_CRYPTOTCP_H
#define CROW_GATES_CRYPTOTCP_H

#include <crow/defs.h>
#include <crow/gateway.h>
#include <iostream>

namespace crow
{

	class crypto_tcp_gate : public gateway
	{
		int sock = 0;
		crow::packet *block = nullptr;

	public:
		crypto_tcp_gate(){}
		crypto_tcp_gate(uint16_t port) 
		{
			open(port); 
		}

		void send(crow::packet *) override;
		void nblock_onestep() override;

		int open(uint16_t port);
		void finish();

#if CROW_ENABLE_WITHOUT_FDS
#else
		int get_fd() { return sock; }
#endif

	};

	udpgate *create_crypto_tcp_gate(uint8_t id, uint16_t port);
} // namespace crow

#endif
