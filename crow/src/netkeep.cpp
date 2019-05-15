#include <crow/netkeep.h>
#include <crow/tower.h>

void crow::netkeep_protocol_handler_stub(crow::packet * pack) 
{
	crow::release(pack);
}

void (*crow::netkeep_protocol_handler)(crow::packet * pack) = 
	crow::netkeep_protocol_handler_stub;
