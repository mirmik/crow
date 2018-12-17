#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

void crow::udpgate::nblock_onestep()
{
	if (!block) block =
		(crow::packet*) malloc(128 + sizeof(crow::packet) - sizeof(crow::header));

	struct sockaddr_in sender;
	socklen_t sendsize = sizeof(sender);
	memset(&sender, 0, sizeof(sender));

	ssize_t len = recvfrom(sock, &block->header, 128, 0, (struct sockaddr*) &sender, &sendsize);
	if (len <= 0) return;

	crow::packet_initialization(block, this);

	struct iovec vec[3] =
	{
		{ &id, 1 },
		{ &sender.sin_addr.s_addr, 4 },
		{ &sender.sin_port, 2 }
	};

	block->revert(vec, 3);
	
	crow::packet* pack = block;
	block = NULL;

	crow::travel(pack);
}

int crow::udpgate::open(uint16_t  port)
{
	int ret;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);
	ipaddr.sin_port = htons(port);

	if (port != 0)
	{
		ret = bind(sock, (struct sockaddr*) &ipaddr, iplen);
		if (ret != 0)
		{
			perror("bind");
			exit(-1);
		}
	}

	int flags = fcntl(sock, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);

	return ret;
}

void crow::udpgate::send(crow::packet* pack)
{
	uint32_t* addr = (uint32_t*)(pack->stageptr() + 1);
	uint16_t* port = (uint16_t*)(pack->stageptr() + 5);

	struct sockaddr_in 	ipaddr;
	socklen_t iplen = sizeof(struct sockaddr_in);
	memset(&ipaddr, 0, iplen);

	//Конвертация не требуется, т.к. crow использует сетевой порядок записи адреса.
	ipaddr.sin_port = *port;
	ipaddr.sin_addr.s_addr = *addr;

	sendto(sock, (const char*)&pack->header, pack->header.flen, 0, (struct sockaddr*)&ipaddr, iplen);
	crow::return_to_tower(pack, CROW_SENDED);
}

crow::udpgate* crow::create_udpgate(uint8_t id, uint16_t port)
{
	crow::udpgate* g = new crow::udpgate;
	g->block = NULL;
	g->open(port); // TODO: should return NULL on error
	crow::link_gate(g, id);
	return g;
}