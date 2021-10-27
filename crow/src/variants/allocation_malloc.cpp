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

void crow_deallocate_packet(crow_packet *pack)
{
    if (pack)
        crow_allocated_count--;
    free(pack);
}

crow_packet *crow_allocate_packet(size_t adlen)
{
    crow_allocated_count++;

    // inc 1 for zero symbol
    crow_packet *ret = (crow_packet *)malloc(adlen + sizeof(crow_packet) + 1);

    return ret;
}
