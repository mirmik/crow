/**
	@file
*/

#ifndef CROW_GATES_SERIAL_GSTUFF_H
#define CROW_GATES_SERIAL_GSTUFF_H

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>

#include <crow/gateway.h>

#include <owl/gstuff/autorecv.h>
#include <owl/gstuff/gstuff.h>

namespace crow 
{
	struct serial_gstuff : public crow::gateway {
		int fd;

		struct crow::packet * rpack;
		bool debug;

		struct gstuff_autorecv recver;

		void newline_handler();

		void send(crow::packet*) override;
		void nblock_onestep() override;
	};

	crow::serial_gstuff* create_serial_gstuff(const char* path, uint32_t baudrate, uint8_t id, bool debug);
}

#endif