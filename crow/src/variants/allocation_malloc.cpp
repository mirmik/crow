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

void crow_deallocate_packet(crow::packet *pack)
{
    if (pack)
        crow_allocated_count--;
    delete pack;
}

crow::packet *crow_allocate_packet(int alen, int dlen)
{
    crow_allocated_count++;

    crow::morph_packet * pack = new crow::morph_packet;
    pack->allocate_buffer(alen, dlen);
    
    return pack;
}
