#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <fcntl.h>

#include <gxx/debug/dprint.h>

void crow_udpgate_nblock_onestep(crow_gw_t* g) {

	crow_udpgate_t* udpgate = mcast_out(g, crow_udpgate_t, gw);
	if (!udpgate->block) udpgate->block = 
		(crow_packet_t*) malloc(128 + sizeof(crow_packet_t) - sizeof(crow_header_t));


	struct sockaddr_in sender;
	socklen_t sendsize = sizeof(sender);
	memset(&sender, 0, sizeof(sender));

	int len = recvfrom(udpgate->sock, &udpgate->block->header, 128, 0, (struct sockaddr*) &sender, &sendsize);
	if (len <= 0) return;

	crow_packet_initialization(udpgate->block, g);

	uint32_t ip = ntohl(sender.sin_port);
	uint16_t port = ntohs(sender.sin_addr.s_addr);
	crow_packet_revert_2(udpgate->block, &ip, 2, &port, 4, G1_UDPGATE);

	crow_packet_t* pack = udpgate->block;
	udpgate->block = NULL;
			
	crow_travel(pack);
}

int crow_udpgate_open(crow_udpgate_t* g, uint16_t  port) {
	g->sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);
	ipaddr.sin_port = htons(port);

	int ret = bind(g->sock, (struct sockaddr*) &ipaddr, iplen);

	int flags = fcntl(g->sock, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(g->sock, F_SETFL, flags);
	
	return ret;
}

crow_gw_t* crow_create_udpgate(uint16_t port, uint8_t id) {
	crow_udpgate_t* g = (crow_udpgate_t*) malloc(sizeof(crow_udpgate_t));
	//crow_gw_init(&g->gw);
	g->block = NULL;
	crow_udpgate_open(g, port); // TODO: should return NULL on error
	crow_link_gate(&g->gw, id);
	g->gw.ops = &crow_udpgate_ops;
	return &g->gw;
}


void crow_udpgate_send(crow_gw_t* g, crow_packet_t* pack)  {
	crow_udpgate_t* udpgate = (crow_udpgate_t*) g;
	uint32_t* addr = (uint32_t*)(crow_packet_stageptr(pack) + 1);
	uint16_t* port = (uint16_t*)(crow_packet_stageptr(pack) + 5);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);

	ipaddr.sin_port = *port;
	ipaddr.sin_addr.s_addr = *addr;

	sendto(udpgate->sock, (const char*)&pack->header, pack->header.flen, 0, (struct sockaddr*)&ipaddr, iplen);
	crow_return_to_tower(pack, CROW_SENDED);
}