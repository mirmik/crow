#include <crow/channel.h>
#include <crow/error.h>

int crow::channel::connect(const uint8_t *raddr, uint16_t rlen, uint16_t rid,
		    uint8_t qos, uint16_t ackquant) 
{
	handshake(raddr, rlen, rid, qos, ackquant);
	int ret = waitevent();

	if (ret)
		return CROW_ERR_UNDELIVERED;

	return 0;
}