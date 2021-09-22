/**
@file packet.cpp
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <crow/gateway.h>
#include <crow/packet.h>
#include <igris/sync/syslock.h>

#include <igris/compiler.h>

void crow_packet_initialization(struct crow_packet *pack, crow::gateway *ingate)
{
    dlist_init(&pack->lnk);
    dlist_init(&pack->ulnk);
    pack->ingate = ingate;
    pack->_ackcount = 5;
    pack->flags = 0;
    pack->refs = 0;
    *((char*)(&pack->header) + pack->header.flen) = 0;
}

struct crow_packet *
crow_create_packet(crow::gateway *ingate, uint8_t addrsize, size_t datasize)
{
    crow_packet *pack = crow_allocate_packet(addrsize + datasize);

    if (pack == nullptr)
        return nullptr;

    pack->header.flen = (uint16_t)(sizeof(crow_header) + addrsize + datasize);
    pack->header.alen = addrsize;
    pack->header.ackquant = 200;
    pack->header.pflag = 0;
    pack->header.qos = 0;
    pack->header.stg = 0;

    crow_packet_initialization(pack, ingate);

    return pack;
}

void crow_packet_revert_gate(struct crow_packet * pack, uint8_t gateindex)
{
    *crow_packet_stageptr(pack) = gateindex;
    ++pack->header.stg;
}

void crow_packet_revert(struct crow_packet * pack, igris::buffer *vec, size_t veclen)
{
    igris::buffer *it = vec + veclen - 1;
    igris::buffer *eit = vec - 1;

    size_t sz = 0;
    uint8_t *tgt = crow_packet_stageptr(pack);

    for (; it != eit; --it)
    {
        sz += it->size();
        char *ptr = (char *)it->data() + it->size();
        char *eptr = (char *)it->data();
        while (ptr != eptr)
            *tgt++ = *--ptr;
    }
    pack->header.stg = (uint8_t)(pack->header.stg + sz);
}

bool crow::has_allocated() { return !!crow_allocated_count; }



uint8_t * crow_packet_addrptr(struct crow_packet * pack)
{
    return (uint8_t *)(&pack->header + 1);
}

uint8_t crow_packet_addrsize(struct crow_packet * pack)
{
    return pack->header.alen;
}

char * crow_packet_dataptr(struct crow_packet * pack)
{
    return (char *)(crow_packet_addrptr(pack) + crow_packet_addrsize(pack));
}

uint16_t crow_packet_datasize(struct crow_packet * pack)
{
    return (uint16_t)(pack->header.flen - pack->header.alen - sizeof(struct crow_header));
}
