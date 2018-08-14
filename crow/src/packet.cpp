/**
@file packet.cpp
*/

#include <crow/packet.h>
#include <gxx/panic.h>
#include <gxx/syslock.h>

crow::packet* crow::create_packet(crow::gateway* ingate, size_t addrsize, size_t datasize) { 
	gxx::system_lock();
	crow::packet* pack = crow::allocate_packet(addrsize + datasize);
	gxx::system_unlock();
	
	pack -> header.flen = sizeof(crow::packet_header) + addrsize + datasize;
	pack -> header.alen = addrsize;
	pack -> header.ackquant = 200;
	pack -> header.pflag = 0;
	pack -> header.qos = (crow::QoS)0;
	pack -> header.stg = 0;

	dlist_init(&pack->lnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
	
	return pack;
}

void crow::packet_initialization(crow::packet* pack, crow::gateway* ingate) { 
	dlist_init(&pack->lnk);
	pack -> ingate = ingate;
	pack -> ackcount = 0; 
	pack -> flags = 0;
}

void crow::utilize(crow::packet* pack) {
	gxx::system_lock();
	dlist_del(&pack->lnk);
	crow::utilize_packet(pack);
	gxx::system_unlock();
}

void crow::packet::revert_stage(void* addr1, uint8_t size1, void* addr2, uint8_t size2, uint8_t gateindex) {
	*(stageptr() + size1 + size2) = gateindex;
	memcpy(stageptr(), addr1, size1);
	memcpy(stageptr() + size1, addr2, size2);
	header.stg += 1 + size1 + size2;
}

void crow::packet::revert_stage(void* addr, uint8_t size, uint8_t gateindex) {
	*(stageptr() + size) = gateindex;
	memcpy(stageptr(), addr, size);
	header.stg += 1 + size;
}

void crow::packet::revert_stage(uint8_t gateindex) {
	*stageptr() = gateindex;
	++header.stg;
}