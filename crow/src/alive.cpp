#include <crow/alive.h>

const char * crow::netname;

void crow::send_alive_message(const uint8_t* raddr, size_t rlen,
							uint8_t code, uint8_t type,
							uint8_t qos, uint16_t ackquant) 
{
	struct alive_header header;

	header.code = code;
	header.type = type; // crowker, или обычная башня
	header.nlen = crow::netname ? strlen(crow::netname) : 0;

	struct iovec iov[] =
	{
		{&header, sizeof(alive_header)},
		{(void *)crow::netname, header.nlen},
	};

	crow::send_v(raddr, rlen, iov, 2,
             	CROW_NETKEEP_PROTOCOL, qos, ackquant);
}
