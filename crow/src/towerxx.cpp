#include <crow/towerxx.h>

crow::packet_ptr crow::send(const std::vector<uint8_t>& addr, const std::string& data,
	uint8_t type, uint8_t qos, uint16_t ackquant) 
{
	return crow::send(addr.data(), addr.size(), data.data(), data.size(), type, qos, ackquant);
}