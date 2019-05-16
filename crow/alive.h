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

	void send_alive(const uint8_t* raddr, size_t rlen,
							uint8_t code, uint8_t type,
							uint8_t qos, uint16_t ackquant);

	void set_netname(const char * name);
	const char * netname();
}

#endif