#ifndef CROW_ALIVE_H
#define CROW_ALIVE_H

#include <stdint.h>
#include <vector>
#include <map>
#include <chrono>

#include <crow/proto/node.h>
#include <crow/tower.h>
#include <crow/defs.h>

/*#define CROW_TOWER_TYPE_COMMON 0
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
}*/

namespace crow 
{
	struct alive_head 
	{
		uint16_t netname_length;
		uint16_t dietime;
	} __attribute__((packed));

	static inline void send_alive(
		const igris::buffer addr, const char* netname, 
		uint16_t dietime,
		uint8_t qos, uint16_t ackquant) 
	{
		alive_head head;

		head.netname_length = strlen(netname);
		head.dietime = dietime;

		igris::buffer bufs[] = 
		{
			{ &head, sizeof(head) },
			{ netname, head.netname_length }
		};

		crow::node_send_v(0, CROWKER_ALIVE_NODE_NO, addr, bufs, 2, qos, ackquant);
	}

	void start_alive(const std::vector<uint8_t>& addr, const char* netname, 
		uint16_t resend_time,  uint16_t dietime,
		uint8_t qos, uint16_t ackquant);

	void stop_alive();

	class alive_node : public crow::node 
	{
		struct record 
		{
			std::chrono::milliseconds lasttime;
			std::string name;
		};
		std::map<std::vector<uint8_t>,record> dict;

		void incoming_packet(crow::packet *pack) override;

		void undelivered_packet(crow::packet *pack) override
		{
			crow::release(pack);
		}
	};
}

#endif