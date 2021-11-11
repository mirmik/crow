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

void crow_packet_initialization(crow::packet *pack, crow::gateway *ingate)
{
    dlist_init(&pack->lnk);
    dlist_init(&pack->ulnk);
    pack->ingate = ingate;
    pack->_ackcount = 5;
    pack->u.flags = 0;
    pack->refs = 0;
    pack->self_init();
}

crow::compacted_packet *crow_create_packet(crow::gateway *ingate, uint8_t addrsize,
                                 size_t datasize)
{
    crow::compacted_packet *pack = crow_allocate_packet(addrsize + datasize);

    if (pack == nullptr)
        return nullptr;

    pack->header().flen = (uint16_t)(sizeof(crow::header_v1) + addrsize + datasize);
    pack->header().alen = addrsize;
    pack->set_ackquant(200);
    pack->header().u.pflag = 0;
    pack->set_quality(0);
    pack->set_stage(0);

    crow_packet_initialization(pack, ingate);

    return pack;
}

void crow::packet::revert_gate(uint8_t gateindex)
{
    *stageptr() = gateindex;
    increment_stage(1);
}

void crow::packet::revert(igris::buffer *vec, size_t veclen)
{
    igris::buffer *it = vec + veclen - 1;
    igris::buffer *eit = vec - 1;

    size_t sz = 0;
    uint8_t *tgt = stageptr();

    for (; it != eit; --it)
    {
        sz += it->size();
        char *ptr = (char *)it->data() + it->size();
        char *eptr = (char *)it->data();
        while (ptr != eptr)
            *tgt++ = *--ptr;
    }
    increment_stage(sz);
}

bool crow::has_allocated() { return !!crow_allocated_count; }

uint8_t *crow::compacted_packet::addrptr()
{
    return (uint8_t *)(&header() + 1);
}

uint8_t crow::compacted_packet::addrsize()
{
    return _header.alen;
}

char *crow::compacted_packet::dataptr()
{
    return (char *)(addrptr() + addrsize());
}

uint16_t crow::compacted_packet::datasize()
{
    return (uint16_t)(full_length() - addrsize() -
                      sizeof(struct crow::header_v1));
}

char *crow::compacted_packet::endptr()
{
    return (char *)&header() + full_length();
}