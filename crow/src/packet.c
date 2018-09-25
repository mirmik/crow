/**
@file packet.cpp
*/

#include <string.h>
#include <stdio.h>

#include <crow/packet.h>
#include <crow/gateway.h>
#include <gxx/syslock.h>

crow_packet_t* crow_create_packet(crow_gw_t* ingate, size_t addrsize, size_t datasize) { 
	system_lock();
	crow_packet_t* pack = crow_allocate_packet(addrsize + datasize);
	system_unlock();
	
	pack -> header.flen = sizeof(crow_header_t) + addrsize + datasize;
	pack -> header.alen = addrsize;
	pack -> header.ackquant = 200;
	pack -> header.pflag = 0;
	pack -> header.qos = 0;
	pack -> header.stg = 0;

	dlist_init(&pack->lnk);
	dlist_init(&pack->ulnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
	
	return pack;
}

void crow_packet_initialization(crow_packet_t* pack, crow_gw_t* ingate) { 
	dlist_init(&pack->lnk);
	dlist_init(&pack->ulnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
}

void crow_utilize(crow_packet_t* pack) {
	system_lock();
	//dlist_del(&pack->lnk); // Очищается в tower_release
	dlist_del(&pack->ulnk);
	crow_deallocate_packet(pack);
	system_unlock();
}

void crow_packet_revert_g(crow_packet_t* pack, uint8_t gateindex) {
	*crow_packet_stageptr(pack) = gateindex;
	++pack->header.stg;
}

void crow_packet_revert(crow_packet_t* pack, struct iovec* vec, size_t veclen) {
	struct iovec* it = vec + veclen - 1;
	struct iovec* eit = vec - 1;

	size_t sz = 0;
	char* tgt = crow_packet_stageptr(pack);

	for (; it != eit; --it) {
		sz += it->iov_len;
		char* ptr = it->iov_base + it->iov_len;
		char* eptr = it->iov_base;
		while(ptr != eptr) 
			*tgt++ = *--ptr;
	}
	pack->header.stg += sz;
}