/**
@file packet.cpp
*/

#include <string.h>

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
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
	
	return pack;
}

void crow_packet_initialization(crow_packet_t* pack, crow_gw_t* ingate) { 
	dlist_init(&pack->lnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
}

void crow_utilize(crow_packet_t* pack) {
	system_lock();
	dlist_del(&pack->lnk);
	crow_deallocate_packet(pack);
	system_unlock();
}

void crow_packet_revert_2(crow_packet_t* pack, void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex) {
	*(crow_packet_stageptr(pack) + size1 + size2) = gateindex;
	memcpy(crow_packet_stageptr(pack), addr1, size1);
	memcpy(crow_packet_stageptr(pack) + size1, addr2, size2);
	pack->header.stg += 1 + size1 + size2;
}

void crow_packet_revert_1(crow_packet_t* pack, void* addr, uint8_t size, uint8_t gateindex) {
	*(crow_packet_stageptr(pack) + size) = gateindex;
	memcpy(crow_packet_stageptr(pack), addr, size);
	pack->header.stg += 1 + size;
}

void crow_packet_revert_g(crow_packet_t* pack, uint8_t gateindex) {
	*crow_packet_stageptr(pack) = gateindex;
	++pack->header.stg;
}