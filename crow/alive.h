#ifndef CROW_ALIVE_H
#define CROW_ALIVE_H

#include <crow/tower.h>

#define CROW_TOWER_TYPE_COMMON 0
#define CROW_TOWER_TYPE_CROWKER 2

#define CROW_ALIVE 7
#define CROW_ALIVE_HANDSHAKE 8

namespace crow
{
	struct alive_header
	{
		uint8_t code;
		uint8_t nlen;
		uint8_t type;
	} __attribute__((packed));

	extern const char * netname;

	void send_alive_message(const uint8_t* raddr, size_t rlen,
							//const char* name, size_t nlen,
							uint8_t code, uint8_t type,
							uint8_t qos, uint16_t ackquant);
}

#endif