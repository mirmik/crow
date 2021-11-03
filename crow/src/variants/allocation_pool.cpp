#include <crow/packet.h>
#include <igris/container/pool.h>

#include <igris/sync/syslock.h>

extern bool __live_diagnostic_enabled;
igris::pool _crow_packet_pool;
int crow_allocated_count = 0;

igris::pool *crow::get_package_pool() { return &_crow_packet_pool; }

void crow::engage_packet_pool(void *zone, size_t zonesize, size_t elsize)
{
    _crow_packet_pool.init(zone, zonesize, elsize);
}

void crow_deallocate_packet(crow_packet *pack)
{
    assert(pack);

    system_lock();

    if (pack)
        crow_allocated_count--;

    _crow_packet_pool.put(pack);
    system_unlock();
}

crow_packet *crow_allocate_packet(size_t adlen)
{
    (void) adlen;
    system_lock();
    void *ret = _crow_packet_pool.get();

    if (ret)
        crow_allocated_count++;

    system_unlock();

    if (__live_diagnostic_enabled)
    {
        debug_print("alloc: ");
        debug_printhex_ptr(ret);
        debug_print("\r\n");
    }

    return (crow_packet *)ret;
}
