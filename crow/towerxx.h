#ifndef CROW_TOWERXX_H
#define CROW_TOWERXX_H

#include <crow/tower.h>

#include <string>
#include <vector>

namespace crow 
{
	packet_ptr send(const std::vector<uint8_t>& addr, const std::string& data, 
		uint8_t type, uint8_t qos, uint16_t ackquant);
}

#endif