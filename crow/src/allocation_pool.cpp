#include <crow/packet.h>
#include <owl/container/pool.h>

#include <owl/syslock.h>

owl::pool _crow_packet_pool;

void crow::engage_packet_pool(void* zone, size_t zonesize, size_t elsize) 
{
	_crow_packet_pool.init(zone, zonesize, elsize);
}

void crow::deallocate_packet(crow::packet* pack) {  
	system_lock();
	_crow_packet_pool.put(pack); 
	system_unlock();
}

crow::packet* crow::allocate_packet(size_t adlen) {  
	system_lock();
	void* ret = _crow_packet_pool.get();
	system_unlock();
	return (crow::packet*) ret;
}