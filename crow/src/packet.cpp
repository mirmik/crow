/**
@file packet.cpp
*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <crow/packet.h>
#include <crow/gateway.h>
#include <gxx/syslock.h>

void crow::packet_initialization(crow::packet* pack, crow::gateway* ingate)
{
	dlist_init(&pack->lnk);
	dlist_init(&pack->ulnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0;
	pack -> flags = 0;
	pack -> refs = 0;
}

crow::packet* crow::create_packet(crow::gateway* ingate, uint8_t addrsize, size_t datasize)
{
	system_lock();
	crow::packet* pack = crow::allocate_packet(addrsize + datasize);
	system_unlock();

	pack -> header.flen = (uint16_t)(sizeof(crow::header) + addrsize + datasize);
	pack -> header.alen = addrsize;
	pack -> header.ackquant = 200;
	pack -> header.pflag = 0;
	pack -> header.qos = 0;
	pack -> header.stg = 0;

	packet_initialization(pack, ingate);

	return pack;
}

void crow::packet::revert_gate(uint8_t gateindex)
{
	*stageptr() = gateindex;
	++header.stg;
}

void crow::packet::revert(struct iovec* vec, size_t veclen)
{
	struct iovec* it = vec + veclen - 1;
	struct iovec* eit = vec - 1;

	size_t sz = 0;
	uint8_t* tgt = stageptr();

	for (; it != eit; --it)
	{
		sz += it->iov_len;
		char* ptr = (char*)it->iov_base + it->iov_len;
		char* eptr = (char*)it->iov_base;
		while (ptr != eptr)
			*tgt++ = *--ptr;
	}
	header.stg = (uint8_t)(header.stg + sz);
}