/** @file */

#include <assert.h>
#include <limits>
#include <stdbool.h>
#include <string.h>

#include <igris/dprint.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>
#include <igris/time/systime.h>

#include <crow/gateway.h>
#include <crow/print.h>
#include <crow/proto/node.h>
#include <crow/pubsub/pubsub.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/warn.h>

#include <nos/print.h>

#ifdef CROW_USE_ASYNCIO
#include <crow/asyncio.h>
#endif

// ============================================================================
// Tower class implementation
// ============================================================================

crow::Tower::Tower()
{
    // Initialize TIME_WAIT hash table buckets
    for (size_t i = 0; i < TIME_WAIT_HTABLE_SIZE; ++i)
        dlist_init(&_time_wait_htable[i]);
}

crow::Tower::~Tower() = default;

void crow::Tower::time_wait_init()
{
    if (_time_wait_initialized)
        return;
    for (size_t i = 0; i < MAX_TIME_WAIT_ENTRIES; ++i)
    {
        dlist_init(&_time_wait_pool[i].lnk);
        dlist_init(&_time_wait_pool[i].hlnk);
        dlist_add(&_time_wait_pool[i].lnk, &_time_wait_free_list);
    }
    _time_wait_initialized = true;
}

size_t crow::Tower::time_wait_hash(uint16_t seqid)
{
    return seqid % TIME_WAIT_HTABLE_SIZE;
}

void crow::Tower::time_wait_cleanup_expired()
{
    uint64_t now = igris::millis();
    dlist_head *it, *nxt;
    dlist_for_each_safe(it, nxt, &_time_wait_list)
    {
        time_wait_entry *entry = dlist_entry(it, time_wait_entry, lnk);
        if (now >= entry->expire_time)
        {
            dlist_del(&entry->lnk);
            dlist_del(&entry->hlnk);
            dlist_add(&entry->lnk, &_time_wait_free_list);
            --_time_wait_count;
        }
        else
        {
            // List is ordered by expire time, so we can stop here
            break;
        }
    }
}

bool crow::Tower::time_wait_check(uint16_t seqid,
                                  const uint8_t *addr,
                                  uint8_t addrsize)
{
    time_wait_init();
    time_wait_cleanup_expired();

    dlist_head *bucket = &_time_wait_htable[time_wait_hash(seqid)];
    time_wait_entry *entry;
    dlist_for_each_entry(entry, bucket, hlnk)
    {
        if (entry->seqid == seqid && entry->addrsize == addrsize &&
            memcmp(entry->addr, addr, addrsize) == 0)
        {
            return true; // Found in TIME_WAIT - this is a duplicate
        }
    }
    return false;
}

void crow::Tower::time_wait_add(uint16_t seqid,
                                const uint8_t *addr,
                                uint8_t addrsize)
{
    time_wait_init();
    time_wait_cleanup_expired();

    if (addrsize > sizeof(time_wait_entry::addr))
        return; // Address too long

    // Get entry from free list or evict oldest
    time_wait_entry *entry;
    if (!dlist_empty(&_time_wait_free_list))
    {
        entry = dlist_first_entry(&_time_wait_free_list, time_wait_entry, lnk);
        dlist_del(&entry->lnk);
    }
    else if (!dlist_empty(&_time_wait_list))
    {
        // Evict oldest entry
        entry = dlist_first_entry(&_time_wait_list, time_wait_entry, lnk);
        dlist_del(&entry->lnk);
        dlist_del(&entry->hlnk);
        --_time_wait_count;
    }
    else
    {
        return; // No entries available (shouldn't happen)
    }

    entry->seqid = seqid;
    entry->addrsize = addrsize;
    memcpy(entry->addr, addr, addrsize);
    entry->expire_time = igris::millis() + _time_wait_duration_ms;

    // Add to end of list (ordered by expire time)
    dlist_add_tail(&entry->lnk, &_time_wait_list);
    // Add to hash table
    dlist_add(&entry->hlnk, &_time_wait_htable[time_wait_hash(seqid)]);
    ++_time_wait_count;
}

void crow::Tower::utilize(crow::packet *pack)
{
    dlist_del(&pack->lnk);
    dlist_del(&pack->ulnk);
    pack->invalidate();
}

void crow::Tower::release(crow::packet *pack)
{
    assert(pack);
    system_lock();

    if (pack->u.f.released_by_tower && pack->refs == 0)
        utilize(pack);
    else
        pack->u.f.released_by_world = true;

    system_unlock();
}

void crow::Tower::tower_release(crow::packet *pack)
{
    system_lock();

    dlist_del_init(&pack->lnk);

    // Remove from hash table if present (just disconnect the intrusive link)
    // We use dlist_del_init directly instead of _outters_htable.remove()
    // to avoid dependency on specific Tower instance - the link can safely
    // disconnect itself from any hash table bucket it belongs to.
    if (!dlist_empty(&pack->hlnk))
        dlist_del_init(&pack->hlnk);

    if (pack->u.f.released_by_world && pack->refs == 0)
        utilize(pack);
    else
        pack->u.f.released_by_tower = true;

    system_unlock();
}

void crow::Tower::confirmed_utilize_from_outers(crow::packet *pack)
{
    crow::packet *it;
    system_lock();
    // Use hash table for O(1) bucket lookup instead of O(n) list scan
    dlist_head *bucket = _outters_htable.bucket(pack->seqid());
    dlist_for_each_entry(it, bucket, hlnk)
    {
        if (it->seqid() == pack->seqid() &&
            pack->addrsize() == it->addrsize() &&
            !memcmp(it->addrptr(), pack->addrptr(), pack->addrsize()))
        {
            if (_diagnostic_enabled)
            {
                dpr("OUTFLT: ACK received, removing seqid=");
                dprhex(pack->seqid());
                dpr(" from outters, out_count=");
                dpr(_outters_count);
                dln();
            }
            it->u.f.confirmed = 1;
            _outters_htable.remove(&it->hlnk);
            if (_outters_count > 0)
                --_outters_count;
            system_unlock();
            crow::node_protocol.delivered(it, *this);
            tower_release(it);
            return;
        }
    }
    if (_diagnostic_enabled)
    {
        dpr("OUTFLT: ACK for seqid=");
        dprhex(pack->seqid());
        dpr(" but NOT FOUND in outters!\n");
    }
    system_unlock();
}

void crow::Tower::qos_release_from_incoming(crow::packet *pack)
{
    crow::packet *it;
    system_lock();
    // Use hash table for O(1) bucket lookup instead of O(n) list scan
    dlist_head *bucket = _incoming_htable.bucket(pack->seqid());
    dlist_for_each_entry(it, bucket, ihlnk)
    {
        if (it->seqid() == pack->seqid() &&
            pack->addrsize() == it->addrsize() &&
            !memcmp(it->addrptr(), pack->addrptr(), pack->addrsize()))
        {
            if (_diagnostic_enabled)
            {
                dpr("DUPFLT: ACK22 removing seqid=");
                dprhex(pack->seqid());
                dpr(" from htable -> TIME_WAIT, inc_count=");
                dpr(_incoming_count);
                dln();
            }
            // Add to TIME_WAIT before removing from incoming
            time_wait_add(it->seqid(), it->addrptr(), it->addrsize());

            _incoming_htable.remove(&it->ihlnk);
            if (_incoming_count > 0)
                --_incoming_count;
            system_unlock();
            tower_release(it);
            return;
        }
    }
    system_unlock();
}

static bool crow_time_comparator(crow::packet *a, crow::packet *b)
{
    uint64_t a_timer = a->last_request_time + a->ackquant();
    uint64_t b_timer = b->last_request_time + b->ackquant();
    return a_timer < b_timer;
}

bool crow::Tower::add_to_incoming_list(crow::packet *pack)
{
    // Check limit before adding
    if (_incoming_count >= MAX_INCOMING)
    {
        if (_diagnostic_enabled)
        {
            dpr("crow: incoming list full (");
            dpr(_incoming_count);
            dpr("/");
            dpr(MAX_INCOMING);
            dpr("), dropping seqid=");
            dprhex(pack->seqid());
            dln();
        }
        return false;
    }
    pack->last_request_time = igris::millis();
    dlist_move_sorted(pack, &_incoming, lnk, crow_time_comparator);
    // Add to hash table for fast lookup by seqid
    _incoming_htable.put(pack, &pack->ihlnk, pack->seqid());
    ++_incoming_count;
    unsleep();
    return true;
}

bool crow::Tower::add_to_outters_list(crow::packet *pack)
{
    // Check limit before adding (but allow retransmits - already in list)
    bool is_retransmit = !dlist_empty(&pack->hlnk);
    if (!is_retransmit && _outters_count >= MAX_OUTTERS)
    {
        if (_diagnostic_enabled)
        {
            dpr("crow: outters list full (");
            dpr(_outters_count);
            dpr("/");
            dpr(MAX_OUTTERS);
            dpr("), dropping seqid=");
            dprhex(pack->seqid());
            dln();
        }
        return false;
    }
    pack->last_request_time = igris::millis();
    dlist_move_sorted(pack, &_outters, lnk, crow_time_comparator);
    // Add to hash table for fast lookup by seqid
    // First remove if already in table (for retransmits)
    if (is_retransmit)
        _outters_htable.remove(&pack->hlnk);
    else
        ++_outters_count;
    _outters_htable.put(pack, &pack->hlnk, pack->seqid());
    unsleep();
    return true;
}

crow::packet_ptr crow::Tower::travel(crow::packet *pack)
{
    system_lock();
    dlist_add_tail(&pack->lnk, &_travelled);
    system_unlock();
    unsleep();
    return crow::packet_ptr(pack);
}

void crow::Tower::unsleep()
{
    if (_unsleep_handler)
    {
        _unsleep_handler();
    }

#ifdef CROW_USE_ASYNCIO
    crow::asyncio.unsleep();
#endif
}

void crow::Tower::incoming_handler(crow::packet *pack)
{
    if (CROW_NODE_PROTOCOL == pack->type())
    {
        _in_incoming_handler = true;
        crow::node_protocol.incoming(pack, *this);
        _in_incoming_handler = false;
        return;
    }

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    if (CROW_PUBSUB_PROTOCOL == pack->type())
    {
        _in_incoming_handler = true;
        crow::pubsub_protocol.incoming(pack, *this);
        _in_incoming_handler = false;
        return;
    }
#endif

    if (_default_incoming_handler)
    {
        _in_incoming_handler = true;
        _default_incoming_handler(pack);
        _in_incoming_handler = false;
        return;
    }
    else
    {
        if (_diagnostic_enabled)
        {
            crow::diagnostic("wproto", pack, _diagnostic_label, _debug_data_size);
        }
        release(pack);
        return;
    }
}

void crow::Tower::send_ack(crow::packet *pack)
{
    crow::packet *ack = crow::create_packet(NULL, pack->addrsize(), 0);

    assert(pack);
    assert(ack);

    ack->set_type(pack->quality() == CROW_BINARY_ACK ? G1_ACK21_TYPE
                                                     : G1_ACK_TYPE);
    ack->set_ack(1);
    ack->set_quality(CROW_WITHOUT_ACK);
    ack->set_ackquant(pack->ackquant());
    ack->set_seqid(pack->seqid());
    memcpy(ack->addrptr(), pack->addrptr(), pack->addrsize());
    ack->u.f.released_by_world = true;
    travel(ack);
}

void crow::Tower::send_ack2(crow::packet *pack)
{
    crow::packet *ack = crow::create_packet(NULL, pack->addrsize(), 0);

    assert(pack);
    assert(ack);

    ack->set_type(G1_ACK22_TYPE);
    ack->set_ack(1);
    ack->set_quality(CROW_WITHOUT_ACK);
    ack->set_ackquant(pack->ackquant());
    ack->set_seqid(pack->seqid());
    memcpy(ack->addrptr(), pack->addrptr(), pack->addrsize());
    travel(ack);
}

void crow::Tower::revert_address(crow::packet *pack)
{
    uint8_t *first = pack->addrptr();
    uint8_t *last = pack->addrptr() + pack->addrsize();

    while ((first != last) && (first != --last))
    {
        char tmp = *last;
        *last = *first;
        *first++ = tmp;
    }
}

void crow::Tower::send_to_gate_phase(crow::packet *pack)
{
    uint8_t gidx = *pack->stageptr();
    crow::gateway *gate = get_gateway(gidx);

    if (gate == NULL)
    {
        // second version compatible
        struct crow_gateway *gateway = get_c_gateway(gidx);

        if (gateway)
        {
            if (pack->ack())
            {
                if (_diagnostic_enabled && _diagnostic_noack == false)
                    crow::diagnostic("track", pack, _diagnostic_label, _debug_data_size);
            }
            else
            {
                if (_diagnostic_enabled)
                    crow::diagnostic("trans", pack, _diagnostic_label, _debug_data_size);
            }
            assert(pack->u.f.sended_to_gate == 0);
            pack->u.f.sended_to_gate = 1;
            gateway->ops->send(gateway, pack);
            return;
        }

        if (_diagnostic_enabled)
            crow::diagnostic("wgate", pack, _diagnostic_label, _debug_data_size);

        if (pack->ingate == nullptr)
        {
            crow::warn("wrong gate in out packet");
        }

        system_lock();
        utilize(pack);
        system_unlock();
    }
    else
    {
        if (pack->ack())
        {
            if (_diagnostic_enabled && _diagnostic_noack == false)
                crow::diagnostic("track", pack, _diagnostic_label, _debug_data_size);
        }
        else
        {
            if (_diagnostic_enabled)
                crow::diagnostic("trans", pack, _diagnostic_label, _debug_data_size);
        }
        assert(pack->u.f.sended_to_gate == 0);
        pack->u.f.sended_to_gate = 1;
        gate->send(pack);
    }
}

void crow::Tower::incoming_ack_phase(crow::packet *pack)
{
    if (_diagnostic_enabled && _diagnostic_noack == false)
        crow::diagnostic("inack", pack, _diagnostic_label, _debug_data_size);

    switch (pack->type())
    {
        case G1_ACK_TYPE:
            confirmed_utilize_from_outers(pack);
            break;

        case G1_ACK21_TYPE:
            confirmed_utilize_from_outers(pack);
            send_ack2(pack);
            break;

        case G1_ACK22_TYPE:
            qos_release_from_incoming(pack);
            break;

        default:
            break;
    }

    system_lock();
    utilize(pack);
    system_unlock();
}

void crow::Tower::do_travel(crow::packet *pack)
{
    _total_travelled++;

    if (pack->stage() == pack->addrsize())
    {
        // Delivered packet branch
        revert_address(pack);

        if (pack->ack())
        {
            // Intercept ack packets
            incoming_ack_phase(pack);
            return;
        }

        if (_diagnostic_enabled)
            crow::diagnostic("incom", pack, _diagnostic_label, _debug_data_size);

        if (pack->ingate)
        {
            // If packet came from outside, use quality assurance logic
            if (pack->quality() == CROW_TARGET_ACK ||
                pack->quality() == CROW_BINARY_ACK)
                send_ack(pack);

            if (pack->quality() == CROW_BINARY_ACK)
            {
                // Check if packet is in incoming list
                crow::packet *inc;
                dlist_head *bucket = _incoming_htable.bucket(pack->seqid());
                dlist_for_each_entry(inc, bucket, ihlnk)
                {
                    if (inc->seqid() == pack->seqid() &&
                        inc->addrsize() == pack->addrsize() &&
                        memcmp(inc->addrptr(), pack->addrptr(),
                               inc->addrsize()) == 0)
                    {
                        if (_diagnostic_enabled)
                        {
                            dpr("DUPFLT: filtered duplicate seqid=");
                            dprhex(pack->seqid());
                            dpr(" (in incoming_htable)\n");
                        }
                        system_lock();
                        utilize(pack);
                        system_unlock();
                        return;
                    }
                }

                // Check TIME_WAIT for late retransmits after ACK22
                if (time_wait_check(pack->seqid(), pack->addrptr(),
                                    pack->addrsize()))
                {
                    if (_diagnostic_enabled)
                    {
                        dpr("DUPFLT: filtered duplicate seqid=");
                        dprhex(pack->seqid());
                        dpr(" (in TIME_WAIT)\n");
                    }
                    system_lock();
                    utilize(pack);
                    system_unlock();
                    return;
                }

                if (_diagnostic_enabled)
                {
                    dpr("DUPFLT: new packet seqid=");
                    dprhex(pack->seqid());
                    dpr(" adding to htable, inc_count=");
                    dpr(_incoming_count);
                    dln();
                }

                // Mark packet as received for filtering subsequent copies
                system_lock();
                if (!add_to_incoming_list(pack))
                {
                    // List full, drop packet
                    utilize(pack);
                    system_unlock();
                    return;
                }
                system_unlock();
            }
        }

        if (pack->ingate && pack->quality() != 2)
        {
            tower_release(pack);
        }

        // Process incoming packet
        incoming_handler(pack);
        return;
    }
    else
    {
        // Transit packet branch
        if (pack->ingate && _retransling_allowed == false)
        {
            static int warned = 0;
            if (warned == 0)
            {
                warned = 1;
#ifndef MEMORY_ECONOMY
                crow::warn(
                    "Crow get retransling request but retransling is not "
                    "allowed.\n"
                    "Use tower.set_retransling_allowed(true) to enable "
                    "retransling\n"
                    "Or use --retransler option if utility allowed it.\n");
#endif
            }

            system_lock();
            utilize(pack);
            system_unlock();
            return;
        }

        send_to_gate_phase(pack);
    }
}

crow::packet_ptr crow::Tower::transport(crow::packet *pack, bool async)
{
    pack->set_stage(0);
    pack->set_ack(0);
    system_lock();
    pack->set_seqid(_seqcounter++);
    system_unlock();

    crow::packet_ptr ptr(pack);

    if (async)
    {
        return travel(pack);
    }
    else
    {
        do_travel(pack);
        return ptr;
    }
}

void crow::Tower::nocontrol_travel(crow::packet *pack, bool fastsend)
{
    if (fastsend)
    {
        do_travel(pack);
        return;
    }

    system_lock();
    dlist_add_tail(&pack->lnk, &_travelled);
    system_unlock();
}

crow::packet_ptr crow::Tower::send(const crow::hostaddr_view &addr,
                                   const nos::buffer data,
                                   uint8_t type,
                                   uint8_t qos,
                                   uint16_t ackquant,
                                   bool async)
{
    crow::packet *pack = crow::create_packet(NULL, addr.size(), data.size());
    if (pack == nullptr)
    {
        crow::warn("cannot create packet");
        return nullptr;
    }

    pack->set_type(type & 0x1F);
    pack->set_quality(qos);
    pack->set_ackquant(ackquant);

    memcpy(pack->addrptr(), addr.data(), addr.size());
    memcpy(pack->dataptr(), data.data(), data.size());

    return transport(pack, async);
}

crow::packet_ptr crow::Tower::send_v(const crow::hostaddr_view &addr,
                                     const nos::buffer *vec,
                                     size_t veclen,
                                     uint8_t type,
                                     uint8_t qos,
                                     uint16_t ackquant,
                                     bool async)
{
    size_t dsize = 0;
    const nos::buffer *it = vec;
    const nos::buffer *const eit = vec + veclen;

    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    crow::packet *pack = crow::create_packet(NULL, addr.size(), dsize);
    if (pack == nullptr)
        return nullptr;

    pack->set_type(type & 0x1F);
    pack->set_quality(qos);
    pack->set_ackquant(ackquant);

    memcpy(pack->addrptr(), addr.data(), addr.size());

    it = vec;
    char *dst = pack->dataptr();

    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    return transport(pack, async);
}

crow::packet_ptr crow::Tower::send_vv(const crow::hostaddr_view &addr,
                                      const nos::buffer *vec,
                                      size_t veclen,
                                      const nos::buffer *vec2,
                                      size_t veclen2,
                                      uint8_t type,
                                      uint8_t qos,
                                      uint16_t ackquant,
                                      bool async)
{
    size_t dsize = 0;
    const nos::buffer *it;
    const nos::buffer *eit;

    it = vec;
    eit = vec + veclen;
    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    it = vec2;
    eit = vec2 + veclen2;
    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    crow::packet *pack = crow::create_packet(NULL, addr.size(), dsize);
    if (pack == nullptr)
        return nullptr;

    pack->set_type(type & 0x1F);
    pack->set_quality(qos);
    pack->set_ackquant(ackquant);

    memcpy(pack->addrptr(), addr.data(), addr.size());
    char *dst = pack->dataptr();

    it = vec;
    eit = vec + veclen;
    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    it = vec2;
    eit = vec2 + veclen2;
    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    return transport(pack, async);
}

crow::packet_ptr crow::Tower::send_vvv(const crow::hostaddr_view &addr,
                                       const nos::buffer *vec,
                                       size_t veclen,
                                       const nos::buffer *vec2,
                                       size_t veclen2,
                                       const nos::buffer *vec3,
                                       size_t veclen3,
                                       uint8_t type,
                                       uint8_t qos,
                                       uint16_t ackquant,
                                       bool async)
{
    size_t dsize = 0;
    const nos::buffer *it;
    const nos::buffer *eit;

    it = vec;
    eit = vec + veclen;
    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    it = vec2;
    eit = vec2 + veclen2;
    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    it = vec3;
    eit = vec3 + veclen3;
    for (; it != eit; ++it)
    {
        dsize += it->size();
    }

    crow::packet *pack = crow::create_packet(NULL, addr.size(), dsize);
    if (pack == nullptr)
        return nullptr;

    pack->set_type(type & 0x1F);
    pack->set_quality(qos);
    pack->set_ackquant(ackquant);

    memcpy(pack->addrptr(), addr.data(), addr.size());
    char *dst = pack->dataptr();

    it = vec;
    eit = vec + veclen;
    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    it = vec2;
    eit = vec2 + veclen2;
    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    it = vec3;
    eit = vec3 + veclen3;
    for (; it != eit; ++it)
    {
        memcpy(dst, it->data(), it->size());
        dst += it->size();
    }

    return transport(pack, async);
}

void crow::Tower::return_to_tower(crow::packet *pack, uint8_t sts)
{
    pack->u.f.sended_to_gate = 0;
    assert(pack);

    system_lock();

    if (pack->ingate != NULL)
    {
        // Packet was sent and it's not from here. Destroy.
        utilize(pack);
    }
    else
    {
        // Local packet
        if (sts != CROW_SENDED || pack->quality() == CROW_WITHOUT_ACK)
            tower_release(pack);
        else if (!add_to_outters_list(pack))
        {
            // List full, mark as undelivered
            pack->u.f.undelivered = 1;
            tower_release(pack);
        }
    }

    system_unlock();
}

void crow::Tower::undelivered(crow::packet *pack)
{
    pack->u.f.undelivered = 1;

    if (_undelivered_handler)
    {
        _in_undelivered_handler = true;
        _undelivered_handler(pack);
        _in_undelivered_handler = false;
    }

    if (CROW_NODE_PROTOCOL == pack->type())
    {
        _in_undelivered_handler = true;
        crow::node_protocol.undelivered(pack, *this);
        _in_undelivered_handler = false;
        return;
    }

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    else if (CROW_PUBSUB_PROTOCOL == pack->type())
    {
        _in_undelivered_handler = true;
        crow::pubsub_protocol.undelivered(pack, *this);
        _in_undelivered_handler = false;
        return;
    }
#endif
}

void crow::Tower::onestep_send_stage()
{
    crow::packet *pack;

    system_lock();

    while (1)
    {
        if (dlist_empty(&_travelled))
        {
            break;
        }

        pack = dlist_first_entry(&_travelled, crow::packet, lnk);
        dlist_del_init(&pack->lnk);

        system_unlock();
        do_travel(pack);

        system_lock();
    }
    system_unlock();
}

void crow::Tower::onestep_outers_stage()
{
    crow::packet *pack;
    crow::packet *n;

    uint16_t curtime = igris::millis();

    system_lock();
    if (dlist_empty(&_outters))
    {
        system_unlock();
        return;
    }

    dlist_for_each_entry_safe(pack, n, &_outters, lnk)
    {
        assert(pack->u.f.released_by_tower == 0);

        if (curtime - pack->last_request_time >= pack->ackquant())
        {
            dlist_del_init(&pack->lnk);

            if (pack->_ackcount != 0xFFFF)
                --pack->_ackcount;

            if (pack->_ackcount == 0)
            {
                if (_outters_count > 0)
                    --_outters_count;
                undelivered(pack);
                tower_release(pack);
            }
            else
            {
                if (_diagnostic_enabled)
                {
                    dpr("OUTFLT: RETRANSMIT seqid=");
                    dprhex(pack->seqid());
                    dpr(" ackcount=");
                    dpr(pack->_ackcount);
                    dln();
                }
                system_unlock();
                travel(pack);
                system_lock();
            }
        }
    }
    system_unlock();
}

void crow::Tower::onestep_incoming_stage()
{
    crow::packet *pack;
    crow::packet *n;

    uint16_t curtime = igris::millis();

    system_lock();
    if (dlist_empty(&_incoming))
    {
        system_unlock();
        return;
    }

    dlist_for_each_entry_safe(pack, n, &_incoming, lnk)
    {
        assert(pack->u.f.released_by_tower == 0);

        if (curtime - pack->last_request_time >= pack->ackquant())
        {
            dlist_del_init(&pack->lnk);

            if (pack->_ackcount != 0xFFFF)
                --pack->_ackcount;

            if (pack->_ackcount == 0)
            {
                if (_diagnostic_enabled)
                {
                    dpr("DUPFLT: TIMEOUT removing seqid=");
                    dprhex(pack->seqid());
                    dpr(" from htable, inc_count=");
                    dpr(_incoming_count);
                    dln();
                }
                _incoming_htable.remove(&pack->ihlnk);
                if (_incoming_count > 0)
                    --_incoming_count;
                pack->u.f.undelivered = 1;
                pack->u.f.released_by_tower = true;
                if (pack->u.f.released_by_world && pack->refs == 0)
                    utilize(pack);
            }
            else
            {
                dlist_move_sorted(pack, &_incoming, lnk, crow_time_comparator);
                pack->last_request_time = curtime;
                system_unlock();
                send_ack(pack);
                system_lock();
            }
        }
    }
    system_unlock();
}

void crow::Tower::onestep_keepalive_stage()
{
    crow::keepalive_timer_manager.exec(igris::millis());
}

void crow::Tower::onestep()
{
#ifndef CROW_USE_ASYNCIO
    crow::gateway *gate;
    dlist_for_each_entry(gate, &_gateway_list, lnk) { gate->nblock_onestep(); }
#endif

    onestep_send_stage();
    onestep_keepalive_stage();
    onestep_outers_stage();
    onestep_incoming_stage();
}

void crow::Tower::onestep_travel_only()
{
    onestep_send_stage();
}

int crow::Tower::incomming_stage_count()
{
    return dlist_size(&_incoming);
}

int crow::Tower::outers_stage_count()
{
    return dlist_size(&_outters);
}

bool crow::Tower::has_untravelled()
{
    system_lock();
    int ret = !(dlist_empty(&_travelled) && dlist_empty(&_outters) &&
                dlist_empty(&_incoming));
    system_unlock();
    return ret;
}

bool crow::Tower::has_untravelled_now()
{
    system_lock();
    int ret = !dlist_empty(&_travelled);
    system_unlock();
    return ret;
}

void crow::Tower::finish()
{
    crow::gateway *gate;
    dlist_for_each_entry(gate, &_gateway_list, lnk) { gate->finish(); }
}

void crow::Tower::reset_for_test()
{
    system_lock();

    // Clear travelled list
    while (!dlist_empty(&_travelled))
    {
        crow::packet *pack =
            dlist_first_entry(&_travelled, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    // Clear incoming list
    while (!dlist_empty(&_incoming))
    {
        crow::packet *pack = dlist_first_entry(&_incoming, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    // Clear outters list
    while (!dlist_empty(&_outters))
    {
        crow::packet *pack = dlist_first_entry(&_outters, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    _total_travelled = 0;
    crow::reset_allocated_count();

    system_unlock();
}

bool crow::Tower::fully_empty()
{
    system_lock();
    int ret = dlist_empty(&_travelled) && dlist_empty(&_incoming) &&
              dlist_empty(&_outters);
    system_unlock();
    return ret;
}

int64_t crow::Tower::get_minimal_timeout()
{
    int64_t mininterval = std::numeric_limits<int64_t>::max();
    int64_t curtime = igris::millis();

    auto update_candidate = [&](int64_t candidate) {
        if (mininterval > candidate)
            mininterval = candidate;
    };

    if (!keepalive_timer_manager.empty())
    {
        update_candidate(keepalive_timer_manager.minimal_interval(curtime));
    }

    if (!dlist_empty(&_incoming))
    {
        crow::packet *i = dlist_first_entry(&_incoming, crow::packet, lnk);
        update_candidate(i->last_request_time + i->ackquant() - curtime);
    }

    if (!dlist_empty(&_outters))
    {
        crow::packet *o = dlist_first_entry(&_outters, crow::packet, lnk);
        update_candidate(o->last_request_time + o->ackquant() - curtime);
    }

    if (mininterval == std::numeric_limits<int64_t>::max())
        return -1;
    else if (mininterval < 0)
        return 0;
    else
        return mininterval;
}

void crow::Tower::enable_diagnostic()
{
    _diagnostic_enabled = true;
}

bool crow::Tower::diagnostic_enabled() const
{
    return _diagnostic_enabled;
}

void crow::Tower::diagnostic_setup(bool en)
{
    _diagnostic_enabled = en;
}

void crow::Tower::set_time_wait_duration(uint32_t duration_ms)
{
    _time_wait_duration_ms = duration_ms;
}

uint32_t crow::Tower::get_time_wait_duration() const
{
    return _time_wait_duration_ms;
}

void crow::Tower::set_initial_seqid(uint16_t seqid)
{
    _seqcounter = seqid;
}

uint16_t crow::Tower::get_current_seqid() const
{
    return _seqcounter;
}

void crow::Tower::set_diagnostic_label(const std::string &name)
{
    _diagnostic_label = name;
}

const std::string &crow::Tower::get_diagnostic_label() const
{
    return _diagnostic_label;
}

void crow::Tower::set_diagnostic_noack(bool value)
{
    _diagnostic_noack = value;
}

bool crow::Tower::get_diagnostic_noack() const
{
    return _diagnostic_noack;
}

void crow::Tower::set_debug_data_size(uint16_t size)
{
    _debug_data_size = size;
}

uint16_t crow::Tower::get_debug_data_size() const
{
    return _debug_data_size;
}

void crow::Tower::set_retransling_allowed(bool value)
{
    _retransling_allowed = value;
}

bool crow::Tower::get_retransling_allowed() const
{
    return _retransling_allowed;
}

unsigned int crow::Tower::get_total_travelled() const
{
    return _total_travelled;
}

void crow::Tower::reset_total_travelled()
{
    _total_travelled = 0;
}

void crow::Tower::set_default_incoming_handler(void (*handler)(packet *))
{
    _default_incoming_handler = handler;
}

void (*crow::Tower::get_default_incoming_handler() const)(packet *)
{
    return _default_incoming_handler;
}

void crow::Tower::set_undelivered_handler(void (*handler)(packet *))
{
    _undelivered_handler = handler;
}

void (*crow::Tower::get_undelivered_handler() const)(packet *)
{
    return _undelivered_handler;
}

void crow::Tower::set_unsleep_handler(igris::delegate<void> handler)
{
    _unsleep_handler = handler;
}

igris::delegate<void> crow::Tower::get_unsleep_handler() const
{
    return _unsleep_handler;
}

dlist_head &crow::Tower::gateway_list()
{
    return _gateway_list;
}

dlist_head &crow::Tower::c_gateway_list()
{
    return _c_gateway_list;
}

crow::gateway *crow::Tower::get_gateway(int no)
{
    crow::gateway *g;
    dlist_for_each_entry(g, &_gateway_list, lnk)
    {
        if (g->id == no)
            return g;
    }
    return nullptr;
}

crow_gateway *crow::Tower::get_c_gateway(int no)
{
    crow_gateway *g;
    dlist_for_each_entry(g, &_c_gateway_list, lnk)
    {
        if (g->id == no)
            return g;
    }
    return nullptr;
}

int crow::Tower::bind_gateway(gateway *gate, int gateno)
{
    system_lock();
    if (get_gateway(gateno))
    {
        system_unlock();
        return -1;
    }
    gate->id = gateno;
    dlist_add_tail(&gate->lnk, &_gateway_list);
    system_unlock();
    return 0;
}

int crow::Tower::bind_c_gateway(crow_gateway *gate, int gateno)
{
    system_lock();
    if (get_c_gateway(gateno))
    {
        system_unlock();
        return -1;
    }
    gate->id = gateno;
    dlist_add_tail(&gate->lnk, &_c_gateway_list);
    system_unlock();
    return 0;
}
