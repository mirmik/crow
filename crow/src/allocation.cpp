/**
 * @file
 * @brief Unified packet memory management with runtime allocator selection
 *
 * By default uses malloc. Call crow::engage_packet_pool() to switch to pool mode.
 * Pool mode is useful for embedded systems without dynamic memory.
 */

#include <crow/packet.h>
#include <igris/container/pool.h>
#include <igris/sync/syslock.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

static igris::pool _crow_packet_pool;
static bool _use_pool = false;
static int _crow_allocated_count = 0;

igris::pool *crow::get_package_pool()
{
    return &_crow_packet_pool;
}

void crow::engage_packet_pool(void *zone, size_t zonesize, size_t elsize)
{
    _crow_packet_pool.init(zone, zonesize, elsize);
    _use_pool = true;
}

void crow::disengage_packet_pool()
{
    _use_pool = false;
}

bool crow::is_pool_engaged()
{
    return _use_pool;
}

bool crow::has_allocated()
{
    return _crow_allocated_count > 0;
}

int crow::allocated_count()
{
    return _crow_allocated_count;
}

void crow::reset_allocated_count()
{
    _crow_allocated_count = 0;
}

// Pool-based allocation
static crow::packet *allocate_from_pool(size_t required_size)
{
    assert(_crow_packet_pool.element_size() >= required_size &&
           "Pool element size too small for requested packet");

    system_lock();
    void *mem = _crow_packet_pool.get();
    if (mem)
    {
        _crow_allocated_count++;
        memset(mem, 0, _crow_packet_pool.element_size());
    }
    system_unlock();

    return (crow::packet *)mem;
}

static void deallocate_to_pool(crow::packet *pack)
{
    system_lock();
    _crow_allocated_count--;
    _crow_packet_pool.put(pack);
    system_unlock();

    assert(_crow_allocated_count >= 0);
}

// Malloc-based allocation
static crow::packet *allocate_with_malloc(size_t size)
{
    system_lock();
    _crow_allocated_count++;
    system_unlock();

    assert(_crow_allocated_count < 64);

    uint8_t *buffer = (uint8_t *)malloc(size);
    if (buffer)
        memset(buffer, 0, size);
    return (crow::packet *)buffer;
}

static void deallocate_with_free(crow::packet *pack)
{
    system_lock();
    _crow_allocated_count--;
    system_unlock();

    free((void *)pack);
    assert(_crow_allocated_count >= 0);
}

void crow::deallocate_packet(crow::packet *pack)
{
    if (pack == nullptr)
        return;

    if (_use_pool)
        deallocate_to_pool(pack);
    else
        deallocate_with_free(pack);
}

crow::packet *crow::allocate_packet_header_v1(int adlen)
{
    size_t required = sizeof(crow::packet) + sizeof(crow::header_v1) + adlen;
    uint8_t *buffer;

    if (_use_pool)
        buffer = (uint8_t *)allocate_from_pool(required);
    else
        buffer = (uint8_t *)allocate_with_malloc(required);

    if (buffer == nullptr)
        return nullptr;

    crow::packet *pack = new (buffer) crow::packet(crow::deallocate_packet);
    pack->attach_header((crow::header_v1 *)(buffer + sizeof(crow::packet)));
    pack->attach_addrdata(buffer + sizeof(crow::packet) +
                          sizeof(crow::header_v1));
    return pack;
}

crow::packet *crow::allocate_packet_header_v0(int adlen)
{
    size_t required = sizeof(crow::packet) + sizeof(crow::header_v0) + adlen;
    uint8_t *buffer;

    if (_use_pool)
        buffer = (uint8_t *)allocate_from_pool(required);
    else
        buffer = (uint8_t *)allocate_with_malloc(required);

    if (buffer == nullptr)
        return nullptr;

    crow::packet *pack = new (buffer) crow::packet(crow::deallocate_packet);
    pack->attach_header((crow::header_v0 *)(buffer + sizeof(crow::packet)));
    pack->attach_addrdata(buffer + sizeof(crow::packet) +
                          sizeof(crow::header_v0));
    return pack;
}

crow::packet *crow::allocate_packet_header_v1(int alen, int dlen)
{
    crow::packet *pack = allocate_packet_header_v1(alen + dlen);
    if (pack)
    {
        pack->set_addrsize(alen);
        pack->set_datasize(dlen);
    }
    return pack;
}

crow::packet *crow::allocate_packet_header_v0(int alen, int dlen)
{
    crow::packet *pack = allocate_packet_header_v0(alen + dlen);
    if (pack)
    {
        pack->set_addrsize(alen);
        pack->set_datasize(dlen);
    }
    return pack;
}
