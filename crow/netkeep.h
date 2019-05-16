#ifndef CROW_NETKEEP_H
#define CROW_NETKEEP_H

#include <crow/packet.h>

namespace crow 
{
	void netkeep_protocol_handler_stub(crow::packet * pack);
	void netkeep_protocol_handler_crowker(crow::packet * pack);

	void netkeep_serve();

	extern void (*netkeep_protocol_handler)(crow::packet * pack);
}

#endif