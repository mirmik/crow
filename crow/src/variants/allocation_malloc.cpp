/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <igris/dtrace.h>
#include <igris/sync/syslock.h>
#include <stdlib.h>

void crow::deallocate_packet(crow::packet *pack)
{
    if (pack)
        crow_allocated_count--;
    delete pack;
}

crow::packet *crow::allocate_packet(int alen, int dlen)
{
    crow_allocated_count++;

    crow::morph_packet *pack = new crow::morph_packet;
    pack->allocate_buffer(alen, dlen);
    pack->set_destructor(crow::deallocate_packet);

    return pack;
}

void crow::deallocate_compacted_packet(crow::packet *pack)
{
    if (pack)
        crow_allocated_count--;
    free(pack);
}
