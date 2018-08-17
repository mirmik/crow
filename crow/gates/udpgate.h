/**
	@file
*/

#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <crow/gateway.h>

__BEGIN_DECLS

//#include <gxx/util/hexascii.h>

/*#ifdef __cplusplus
namespace crow {
	struct udpgate : public gateway {
		gxx::inet::udp_socket sock;
		crow::packet* block = nullptr;

		void send(crow::packet* pack) override {
			uint32_t* addr = (uint32_t*)(pack->stageptr() + 1);
			uint16_t* port = (uint16_t*)(pack->stageptr() + 5);
			sock.ne_sendto(*addr, *port, (const char*)&pack->header, pack->header.flen);
			crow::return_to_tower(pack, crow::status::Sended);
		}

		void spin() {
			sock.nonblock(false);
			while(1) {
				crow::packet* pack = (crow::packet*) malloc(128 + sizeof(crow::packet) - sizeof(crow::packet_header));
				gxx::inet::netaddr in;
				int len = sock.recvfrom((char*)&pack->header, 128, &in);
				crow::packet_initialization(pack, this);				
				pack->revert_stage(&in.port, 2, &in.addr, 4, G1_UDPGATE);
				crow::travel(pack);
			
		}
*/

/*
		int open(int port) {
			int ret = sock.bind("0.0.0.0", port);
			if (ret >= 0) sock.nonblock(true);
			return ret;
		}

		int open() {
			sock.nonblock(true);
			return 0;
		}
	};
}
#endif*/

void crow_udpgate_send(crow_gw_t* gw, crow_packet_t* pack);
void crow_udpgate_nblock_onestep(crow_gw_t* gw);

const struct crow_gw_operations crow_udpgate_ops = {
	.send = crow_udpgate_send,
	.nblock_onestep = crow_udpgate_nblock_onestep
};

typedef struct crow_udpgate {
	struct crow_gw gw;
	int sock;
	crow_packet_t* block;	
} crow_udpgate_t;

int crow_udpgate_open(crow_udpgate_t* gw, uint16_t port);
crow_gw_t* crow_create_udpgate(uint16_t port, uint8_t id);

__END_DECLS

#endif