/**
@file packet.cpp
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <crow/gateway.h>
#include <crow/packet.h>
#include <igris/sync/syslock.h>

#include <igris/compiler.h>

void crow::packet_initialization(crow::packet *pack, crow::gateway *ingate)
{
	dlist_init(&pack->lnk);
	dlist_init(&pack->ulnk);
	pack->ingate = ingate;
	pack->ackcount(5);
	pack->flags = 0;
	pack->refs = 0;
}

crow::packet *crow::create_packet(crow::gateway *ingate, uint8_t addrsize,
								  size_t datasize)
{
	system_lock();
	crow::packet *pack = crow::allocate_packet(addrsize + datasize);
	system_unlock();

	if (pack == nullptr)
		return nullptr;

	pack->header.flen = (uint16_t)(sizeof(crow::header) + addrsize + datasize);
	pack->header.alen = addrsize;
	pack->header.ackquant = 200;
	pack->header.pflag = 0;
	pack->header.qos = 0;
	pack->header.stg = 0;

	packet_initialization(pack, ingate);

	return pack;
}

void crow::packet::revert_gate(uint8_t gateindex)
{
	*stageptr() = gateindex;
	++header.stg;
}

void crow::packet::revert(igris::buffer *vec, size_t veclen)
{
	igris::buffer *it = vec + veclen - 1;
	igris::buffer *eit = vec - 1;

	size_t sz = 0;
	uint8_t *tgt = stageptr();

	for (; it != eit; --it)
	{
		sz += it->size();
		char *ptr = (char *)it->data() + it->size();
		char *eptr = (char *)it->data();
		while (ptr != eptr)
			*tgt++ = *--ptr;
	}
	header.stg = (uint8_t)(header.stg + sz);
}


bool crow::has_allocated() 
{
	return !!allocated_count;	
}

crow::packet * crow::make_packet_v(const void *addr, uint8_t asize,
    const igris::buffer *vec, size_t veclen) 
{
	size_t dsize = 0;
	const igris::buffer *it = vec;
	const igris::buffer *const eit = vec + veclen;

	for (; it != eit; ++it)
	{
		dsize += it->size();
	}

	crow::packet *pack = crow::create_packet(NULL, asize, dsize);
	if (pack == nullptr)
		return nullptr;

	//pack->header.f.type = type & 0x1F;
	//pack->header.qos = qos;
	//pack->header.ackquant = ackquant;
	
	memcpy(pack->addrptr(), addr, asize);

	it = vec;
	char *dst = pack->dataptr();

	for (; it != eit; ++it)
	{
		memcpy(dst, it->data(), it->size());
		dst += it->size();
	}

	return pack;
}	