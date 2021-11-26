/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

#include <igris/dtrace.h>
#include <igris/sync/syslock.h>

int crow_allocated_count = 0;

static void crow_deallocate_packet(crow::packet *pack)
{
    if (pack)
        crow_allocated_count--;
    delete pack;
}

crow::packet *crow::allocate_packet(int alen, int dlen)
{
    crow_allocated_count++;

    crow::morph_packet * pack = new crow::morph_packet;
    pack->allocate_buffer(alen, dlen);
    pack->set_destructor(crow_deallocate_packet);
    
    return pack;
}

static void crow_deallocate_compacted_packet(crow::packet *pack)
{
    if (pack)
        crow_allocated_count--;
    free(pack);
}

crow::compacted_packet *crow::allocate_compacted_packet(int alen, int dlen)
{
    return allocate_compacted_packet(alen + dlen);
}

crow::compacted_packet *crow::allocate_compacted_packet(int adlen)
{
    auto buflen = sizeof(crow::compacted_packet) + adlen + 1;
    system_lock();
    void * ret = malloc(buflen);
    memset(ret, 0, buflen);
    auto * pack = new (ret) crow::compacted_packet;

    if (ret)
        crow_allocated_count++;

    pack->set_destructor(crow_deallocate_compacted_packet);
    system_unlock();

    return pack;
}