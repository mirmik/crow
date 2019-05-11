#ifndef CROW_QUERY_H
#define CROW_QUERY_H

#include <crow/packet.h>

namespace crow
{
	void query_protocol_handler(crow::packet * pack);

	crow::packet * send_query(const void *raddr, size_t rlen,
	                          const char* func,
	                          const void * data, size_t dsize,
	                          uint8_t type, uint8_t qos, uint16_t ackquant);

	
}

#endif