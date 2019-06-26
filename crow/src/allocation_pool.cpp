#include <crow/packet.h>
#include <igris/container/pool.h>

#include <igris/sync/syslock.h>

#define NODTRACE 1
#include <igris/dtrace.h>

igris::pool _crow_packet_pool;

void crow::engage_packet_pool(void *zone, size_t zonesize, size_t elsize)
{
	DTRACE();
	_crow_packet_pool.init(zone, zonesize, elsize);
}

void crow::deallocate_packet(crow::packet *pack)
{
	DTRACE();
	system_lock();
	_crow_packet_pool.put(pack);
	system_unlock();
}

crow::packet *crow::allocate_packet(size_t adlen)
{
	DTRACE();
	system_lock();
	void *ret = _crow_packet_pool.get();
	//dprptrln(ret);
	system_unlock();
	return (crow::packet *)ret;
}