#include <crow/alive.h>

static const char * __netname = "";

void crow::send_alive(const uint8_t* raddr, size_t rlen,
							uint8_t code, uint8_t type,
							uint8_t qos, uint16_t ackquant) 
{
	struct alive_header header;

	header.code = code;
	header.type = type; // crowker, или обычная башня
	header.nlen = __netname ? strlen(__netname) : 0;

	struct iovec iov[] =
	{
		{&header, sizeof(alive_header)},
		{(void *)__netname, header.nlen},
	};

	crow::send_v(raddr, rlen, iov, 2,
             	CROW_NETKEEP_PROTOCOL, qos, ackquant);
}


void crow::set_netname(const char * name) 
{
	__netname = name;
}

const char * crow::netname() 
{
	return __netname;
}