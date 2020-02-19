#include <crow/packet.h>
#include <igris/container/pool.h>

#include <igris/sync/syslock.h>

igris::pool _crow_packet_pool;
int crow::allocated_count = 0;

void crow::engage_packet_pool(void *zone, size_t zonesize, size_t elsize)
{
	_crow_packet_pool.init(zone, zonesize, elsize);
}

void crow::deallocate_packet(crow::packet *pack)
{
	system_lock();
	
	if (pack)
		allocated_count--;
	
	_crow_packet_pool.put(pack);
	system_unlock();
}

crow::packet *crow::allocate_packet(size_t adlen)
{
	system_lock();
	void *ret = _crow_packet_pool.get();

	if (ret)
		allocated_count++;
	
	system_unlock();
	return (crow::packet *)ret;
}