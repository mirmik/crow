#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct crow_udpgate
{
	struct crow_gw gw;
	int sock;
	crowket_t* block;
} crow_udpgate_t;

void crow_udpgate_nblock_onestep(crow_gw_t* g)
{
	crow_udpgate_t* udpgate = mcast_out(g, crow_udpgate_t, gw);
	if (!udpgate->block) udpgate->block =
		    (crowket_t*) malloc(128 + sizeof(crowket_t) - sizeof(crow_header_t));

	struct sockaddr_in sender;
	socklen_t sendsize = sizeof(sender);
	memset(&sender, 0, sizeof(sender));

	int len = recvfrom(udpgate->sock, &udpgate->block->header, 128, 0, (struct sockaddr*) &sender, &sendsize);
	if (len <= 0) return;

	crowket_initialization(udpgate->block, g);

	struct iovec vec[3] =
	{
		{ &g->id, 1 },
		{ &sender.sin_addr.s_addr, 4 },
		{ &sender.sin_port, 2 }
	};
	crowket_revert(udpgate->block, vec, 3);

	crowket_t* pack = udpgate->block;
	udpgate->block = NULL;

	crow_travel(pack);
}

int crow_udpgate_open(crow_udpgate_t* g, uint16_t  port)
{
	int ret;

	g->sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);
	ipaddr.sin_port = htons(port);

	if (port != 0)
	{
		ret = bind(g->sock, (struct sockaddr*) &ipaddr, iplen);
		if (ret != 0)
		{
			perror("bind");
			exit(-1);
		}
	}

	int flags = fcntl(g->sock, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(g->sock, F_SETFL, flags);

	return ret;
}

void crow_udpgate_send(crow_gw_t* g, crowket_t* pack)
{
	crow_udpgate_t* udpgate = (crow_udpgate_t*) g;
	uint32_t* addr = (uint32_t*)(crowket_stageptr(pack) + 1);
	uint16_t* port = (uint16_t*)(crowket_stageptr(pack) + 5);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);

	//Конвертация не требуется, т.к. crow использует сетевой порядок записи адреса.
	ipaddr.sin_port = *port;
	ipaddr.sin_addr.s_addr = *addr;

	sendto(udpgate->sock, (const char*)&pack->header, pack->header.flen, 0, (struct sockaddr*)&ipaddr, iplen);
	crow_return_to_tower(pack, CROW_SENDED);
}

const struct crow_gw_operations crow_udpgate_ops =
{
	.send = crow_udpgate_send,
	.nblock_onestep = crow_udpgate_nblock_onestep
};

crow_gw_t* crow_create_udpgate(uint16_t port, uint8_t id)
{
	crow_udpgate_t* g = (crow_udpgate_t*) malloc(sizeof(crow_udpgate_t));
	g->block = NULL;
	crow_udpgate_open(g, port); // TODO: should return NULL on error
	crow_link_gate(&g->gw, id);
	g->gw.ops = &crow_udpgate_ops;
	return &g->gw;
}