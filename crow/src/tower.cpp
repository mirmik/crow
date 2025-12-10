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
#include <crow/warn.h>
#include <crow/src/packet_htable.h>

#include <nos/print.h>

#ifdef CROW_USE_ASYNCIO
#include <crow/asyncio.h>
#endif

bool crow::diagnostic_noack = false;
uint16_t crow::debug_data_size = 60;
unsigned int crow::total_travelled = 0;

bool crow::retransling_allowed = false;

DLIST_HEAD(crow_travelled);
DLIST_HEAD(crow_incoming);
DLIST_HEAD(crow_outters);

// Hash tables for fast lookup of packets by seqid
static crow::packet_htable crow_outters_htable;
static crow::packet_htable crow_incoming_htable;

// List size limits and counters for DoS protection
static constexpr size_t MAX_OUTTERS = 128;
static constexpr size_t MAX_INCOMING = 128;
static size_t outters_count = 0;
static size_t incoming_count = 0;

// TIME_WAIT mechanism for QOS2 duplicate filtering
// Stores "ghost" entries after ACK22 to filter late retransmits
struct time_wait_entry
{
    dlist_head lnk;        // For linked list
    dlist_head hlnk;       // For hash table
    uint64_t expire_time;  // When this entry expires (milliseconds)
    uint16_t seqid;
    uint8_t addrsize;
    uint8_t addr[32];      // Max address size
};

static constexpr size_t TIME_WAIT_HTABLE_SIZE = 64;
static constexpr size_t MAX_TIME_WAIT_ENTRIES = 256;
static dlist_head time_wait_list = DLIST_HEAD_INIT(time_wait_list);
static dlist_head time_wait_htable[TIME_WAIT_HTABLE_SIZE];
static size_t time_wait_count = 0;
static uint32_t time_wait_duration_ms = 15000; // Default 15 seconds

// Pool for time_wait entries
static time_wait_entry time_wait_pool[MAX_TIME_WAIT_ENTRIES];
static dlist_head time_wait_free_list = DLIST_HEAD_INIT(time_wait_free_list);
static bool time_wait_initialized = false;

static void time_wait_init()
{
    if (time_wait_initialized)
        return;
    for (size_t i = 0; i < TIME_WAIT_HTABLE_SIZE; ++i)
        dlist_init(&time_wait_htable[i]);
    for (size_t i = 0; i < MAX_TIME_WAIT_ENTRIES; ++i)
    {
        dlist_init(&time_wait_pool[i].lnk);
        dlist_init(&time_wait_pool[i].hlnk);
        dlist_add(&time_wait_pool[i].lnk, &time_wait_free_list);
    }
    time_wait_initialized = true;
}

static size_t time_wait_hash(uint16_t seqid)
{
    return seqid % TIME_WAIT_HTABLE_SIZE;
}

static void time_wait_cleanup_expired()
{
    uint64_t now = igris::millis();
    dlist_head *it, *nxt;
    dlist_for_each_safe(it, nxt, &time_wait_list)
    {
        time_wait_entry *entry = dlist_entry(it, time_wait_entry, lnk);
        if (now >= entry->expire_time)
        {
            dlist_del(&entry->lnk);
            dlist_del(&entry->hlnk);
            dlist_add(&entry->lnk, &time_wait_free_list);
            --time_wait_count;
        }
        else
        {
            // List is ordered by expire time, so we can stop here
            break;
        }
    }
}

static bool time_wait_check(uint16_t seqid, const uint8_t *addr, uint8_t addrsize)
{
    time_wait_init();
    time_wait_cleanup_expired();

    dlist_head *bucket = &time_wait_htable[time_wait_hash(seqid)];
    time_wait_entry *entry;
    dlist_for_each_entry(entry, bucket, hlnk)
    {
        if (entry->seqid == seqid &&
            entry->addrsize == addrsize &&
            memcmp(entry->addr, addr, addrsize) == 0)
        {
            return true; // Found in TIME_WAIT - this is a duplicate
        }
    }
    return false;
}

static void time_wait_add(uint16_t seqid, const uint8_t *addr, uint8_t addrsize)
{
    time_wait_init();
    time_wait_cleanup_expired();

    if (addrsize > sizeof(time_wait_entry::addr))
        return; // Address too long

    // Get entry from free list or evict oldest
    time_wait_entry *entry;
    if (!dlist_empty(&time_wait_free_list))
    {
        entry = dlist_first_entry(&time_wait_free_list, time_wait_entry, lnk);
        dlist_del(&entry->lnk);
    }
    else if (!dlist_empty(&time_wait_list))
    {
        // Evict oldest entry
        entry = dlist_first_entry(&time_wait_list, time_wait_entry, lnk);
        dlist_del(&entry->lnk);
        dlist_del(&entry->hlnk);
        --time_wait_count;
    }
    else
    {
        return; // No entries available (shouldn't happen)
    }

    entry->seqid = seqid;
    entry->addrsize = addrsize;
    memcpy(entry->addr, addr, addrsize);
    entry->expire_time = igris::millis() + time_wait_duration_ms;

    // Add to end of list (ordered by expire time)
    dlist_add_tail(&entry->lnk, &time_wait_list);
    // Add to hash table
    dlist_add(&entry->hlnk, &time_wait_htable[time_wait_hash(seqid)]);
    ++time_wait_count;
}

igris::delegate<void> crow::unsleep_handler;
void (*crow::default_incoming_handler)(crow::packet *pack) = nullptr;
void (*crow::undelivered_handler)(crow::packet *pack) = nullptr;

static bool __diagnostic_enabled = false;

bool _in_incoming_handler = false;
bool _in_undelivered_handler = false;

bool crow::diagnostic_enabled()
{
    return __diagnostic_enabled;
}
void crow::enable_diagnostic()
{
    __diagnostic_enabled = true;
}

void crow::diagnostic_setup(bool en)
{
    __diagnostic_enabled = en;
}

void crow::utilize(crow::packet *pack)
{
    dlist_del(&pack->lnk); // Очищается в tower_release((см. tower.c))
    dlist_del(&pack->ulnk);
    pack->invalidate();
}

void crow::release(crow::packet *pack)
{
    assert(pack);
    system_lock();

    if (pack->u.f.released_by_tower && pack->refs == 0)
        crow::utilize(pack);
    else
        pack->u.f.released_by_world = true;

    system_unlock();
}

void crow::tower_release(crow::packet *pack)
{
    system_lock();

    //Скорее всего это поле должно освобождаться без инициализации.
    //Инициализируем для последуещего освобождения в utilize
    dlist_del_init(&pack->lnk);

    // Remove from hash table if present (entry points to itself when not in list)
    if (!dlist_empty(&pack->hlnk))
        crow_outters_htable.remove(&pack->hlnk);

    if (pack->u.f.released_by_world && pack->refs == 0)
        crow::utilize(pack);
    else
        pack->u.f.released_by_tower = true;

    system_unlock();
}

static void confirmed_utilize_from_outers(crow::packet *pack)
{
    crow::packet *it;
    system_lock();
    // Use hash table for O(1) bucket lookup instead of O(n) list scan
    dlist_head *bucket = crow_outters_htable.bucket(pack->seqid());
    dlist_for_each_entry(it, bucket, hlnk)
    {
        if (it->seqid() == pack->seqid() &&
            pack->addrsize() == it->addrsize() &&
            !memcmp(it->addrptr(), pack->addrptr(), pack->addrsize()))
        {
            if (__diagnostic_enabled)
            {
                dpr("OUTFLT: ACK received, removing seqid=");
                dprhex(pack->seqid());
                dpr(" from outters, out_count=");
                dpr(outters_count);
                dln();
            }
            it->u.f.confirmed = 1;
            crow_outters_htable.remove(&it->hlnk);
            if (outters_count > 0)
                --outters_count;
            system_unlock();
            crow::node_protocol.delivered(it);
            crow::tower_release(it);
            return;
        }
    }
    if (__diagnostic_enabled)
    {
        dpr("OUTFLT: ACK for seqid=");
        dprhex(pack->seqid());
        dpr(" but NOT FOUND in outters!\n");
    }
    system_unlock();
}

static void qos_release_from_incoming(crow::packet *pack)
{
    crow::packet *it;
    system_lock();
    // Use hash table for O(1) bucket lookup instead of O(n) list scan
    dlist_head *bucket = crow_incoming_htable.bucket(pack->seqid());
    dlist_for_each_entry(it, bucket, ihlnk)
    {
        if (it->seqid() == pack->seqid() &&
            pack->addrsize() == it->addrsize() &&
            !memcmp(it->addrptr(), pack->addrptr(), pack->addrsize()))
        {
            if (__diagnostic_enabled)
            {
                dpr("DUPFLT: ACK22 removing seqid=");
                dprhex(pack->seqid());
                dpr(" from htable -> TIME_WAIT, inc_count=");
                dpr(incoming_count);
                dln();
            }
            // Add to TIME_WAIT before removing from incoming
            time_wait_add(it->seqid(), it->addrptr(), it->addrsize());

            crow_incoming_htable.remove(&it->ihlnk);
            if (incoming_count > 0)
                --incoming_count;
            system_unlock();
            crow::tower_release(it);
            return;
        }
    }
    system_unlock();
}

bool crow_time_comparator(crow::packet *a, crow::packet *b)
{
    uint64_t a_timer = a->last_request_time + a->ackquant();
    uint64_t b_timer = b->last_request_time + b->ackquant();

    return a_timer < b_timer;
}

static bool add_to_incoming_list(crow::packet *pack)
{
    // Check limit before adding
    if (incoming_count >= MAX_INCOMING)
    {
        if (__diagnostic_enabled)
        {
            dpr("crow: incoming list full (");
            dpr(incoming_count);
            dpr("/");
            dpr(MAX_INCOMING);
            dpr("), dropping seqid=");
            dprhex(pack->seqid());
            dln();
        }
        return false;
    }
    pack->last_request_time = igris::millis();
    dlist_move_sorted(pack, &crow_incoming, lnk, crow_time_comparator);
    // Add to hash table for fast lookup by seqid
    crow_incoming_htable.put(pack, &pack->ihlnk, pack->seqid());
    ++incoming_count;
    crow::unsleep();
    return true;
}

static bool add_to_outters_list(crow::packet *pack)
{
    // Check limit before adding (but allow retransmits - already in list)
    bool is_retransmit = !dlist_empty(&pack->hlnk);
    if (!is_retransmit && outters_count >= MAX_OUTTERS)
    {
        if (__diagnostic_enabled)
        {
            dpr("crow: outters list full (");
            dpr(outters_count);
            dpr("/");
            dpr(MAX_OUTTERS);
            dpr("), dropping seqid=");
            dprhex(pack->seqid());
            dln();
        }
        return false;
    }
    pack->last_request_time = igris::millis();
    dlist_move_sorted(pack, &crow_outters, lnk, crow_time_comparator);
    // Add to hash table for fast lookup by seqid
    // First remove if already in table (for retransmits)
    if (is_retransmit)
        crow_outters_htable.remove(&pack->hlnk);
    else
        ++outters_count;
    crow_outters_htable.put(pack, &pack->hlnk, pack->seqid());
    crow::unsleep();
    return true;
}

crow::packet_ptr crow::travel(crow::packet *pack)
{
    system_lock();
    dlist_add_tail(&pack->lnk, &crow_travelled);
    system_unlock();
    crow::unsleep();
    return crow::packet_ptr(pack);
}

void crow::unsleep()
{
    if (unsleep_handler)
    {
        unsleep_handler();
    }

#ifdef CROW_USE_ASYNCIO
    crow::asyncio.unsleep();
#endif
}

static void crow_incoming_handler(crow::packet *pack)
{
    if (CROW_NODE_PROTOCOL == pack->type())
    {
        _in_incoming_handler = true;
        crow::node_protocol.incoming(pack);
        _in_incoming_handler = false;
        return;
    }

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    if (CROW_PUBSUB_PROTOCOL == pack->type())
    {
        _in_incoming_handler = true;
        crow::pubsub_protocol.incoming(pack);
        _in_incoming_handler = false;
        return;
    }
#endif

    if (crow::default_incoming_handler)
    {
        _in_incoming_handler = true;
        crow::default_incoming_handler(pack);
        _in_incoming_handler = false;
        return;
    }
    else
    {
        if (__diagnostic_enabled)
        {
            crow::diagnostic("wproto", pack);
        }
        crow::release(pack);
        return;
    }
}

static void crow_send_ack(crow::packet *pack)
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
    crow::travel(ack);
}

static void crow_send_ack2(crow::packet *pack)
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
    crow::travel(ack);
}

static void crow_revert_address(crow::packet *pack)
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

static void crow_tower_send_to_gate_phase(crow::packet *pack)
{
    uint8_t gidx = *pack->stageptr();
    crow::gateway *gate = crow::get_gateway(gidx);

    if (gate == NULL)
    {
        // second version compatible
        struct crow_gateway *gateway = crow_get_gateway(gidx);

        if (gateway)
        {
            if (pack->ack())
            {
                if (__diagnostic_enabled && crow::diagnostic_noack == false)
                    crow::diagnostic("track", pack);
            }
            else
            {
                if (__diagnostic_enabled)
                    crow::diagnostic("trans", pack);
            }
            //Здесь пакет штампуется временем отправки и пересылается во врата.
            //Врата должны после пересылки отправить его назад в башню
            //с помощью return_to_tower для контроля качества.
            assert(pack->u.f.sended_to_gate == 0);
            pack->u.f.sended_to_gate = 1;
            gateway->ops->send(gateway, pack);
        }

        if (__diagnostic_enabled)
            crow::diagnostic("wgate", pack);

        if (pack->ingate == nullptr)
        {
            crow::warn("wrong gate in out packet");
        }

        system_lock();
        crow::utilize(pack);
        system_unlock();
    }
    else
    {
        if (pack->ack())
        {
            if (__diagnostic_enabled && crow::diagnostic_noack == false)
                crow::diagnostic("track", pack);
        }
        else
        {
            if (__diagnostic_enabled)
                crow::diagnostic("trans", pack);
        }
        //Здесь пакет штампуется временем отправки и пересылается во врата.
        //Врата должны после пересылки отправить его назад в башню
        //с помощью return_to_tower для контроля качества.
        assert(pack->u.f.sended_to_gate == 0);
        pack->u.f.sended_to_gate = 1;
        gate->send(pack);
    }
}

static void crow_tower_incoming_ack_phase(crow::packet *pack)
{
    if (__diagnostic_enabled && crow::diagnostic_noack == false)
        crow::diagnostic("inack", pack);

    switch (pack->type())
    {
        case G1_ACK_TYPE:
            confirmed_utilize_from_outers(pack);
            break;

        case G1_ACK21_TYPE:
            confirmed_utilize_from_outers(pack);
            crow_send_ack2(pack);
            break;

        case G1_ACK22_TYPE:
            qos_release_from_incoming(pack);
            break;

        default:
            break;
    }

    system_lock();
    crow::utilize(pack);
    system_unlock();
}

static void crow_do_travel(crow::packet *pack)
{
    crow::total_travelled++;

    if (pack->stage() == pack->addrsize())
    {
        //Ветка доставленного пакета.
        crow_revert_address(pack);

        if (pack->ack())
        {
            //Перехватываем ack пакеты.
            crow_tower_incoming_ack_phase(pack);
            return;
        }

        if (__diagnostic_enabled)
            crow::diagnostic("incom", pack);

        if (pack->ingate)
        {
            //Если пакет пришел извне, используем логику обеспечения качества.

            //Для пакетов с подтверждение посылаем ack первого или второго
            //типов.
            if (pack->quality() == CROW_TARGET_ACK ||
                pack->quality() == CROW_BINARY_ACK)
                crow_send_ack(pack);

            if (pack->quality() == CROW_BINARY_ACK)
            {
                //Перед тем как добавить пакет в обработку, проверяем,
                //нет ли его в списке принятых.
                crow::packet *inc;
                dlist_head *bucket = crow_incoming_htable.bucket(pack->seqid());
                dlist_for_each_entry(inc, bucket, ihlnk)
                {
                    if (inc->seqid() == pack->seqid() &&
                        inc->addrsize() == pack->addrsize() &&
                        memcmp(inc->addrptr(), pack->addrptr(),
                               inc->addrsize()) == 0)
                    {
                        // Пакет уже фигурирует как принятый, поэтому
                        // отбрасываем его.
                        if (__diagnostic_enabled)
                        {
                            dpr("DUPFLT: filtered duplicate seqid=");
                            dprhex(pack->seqid());
                            dpr(" (in incoming_htable)\n");
                        }
                        system_lock();
                        crow::utilize(pack);
                        system_unlock();

                        return;
                    }
                }

                // Check TIME_WAIT for late retransmits after ACK22
                if (time_wait_check(pack->seqid(), pack->addrptr(), pack->addrsize()))
                {
                    if (__diagnostic_enabled)
                    {
                        dpr("DUPFLT: filtered duplicate seqid=");
                        dprhex(pack->seqid());
                        dpr(" (in TIME_WAIT)\n");
                    }
                    system_lock();
                    crow::utilize(pack);
                    system_unlock();
                    return;
                }

                if (__diagnostic_enabled)
                {
                    dpr("DUPFLT: new packet seqid=");
                    dprhex(pack->seqid());
                    dpr(" adding to htable, inc_count=");
                    dpr(incoming_count);
                    dln();
                }

                // Фиксирем пакет как принятый для фильтрации
                // возможных последующих копий.
                system_lock();
                if (!add_to_incoming_list(pack))
                {
                    // List full, drop packet
                    crow::utilize(pack);
                    system_unlock();
                    return;
                }
                system_unlock();
            }
        }

        if (pack->ingate && pack->quality() != 2)
        {
            // Если пакет отправлен из данного нода, или не требуется
            // подтверждения второго уровня, обслуживание со стороны башни не
            // требуется. Вносим соответствующую пометку, что по crow_release
            // пакет мог быть удалён.
            crow::tower_release(pack);
        }

        //Решаем, что делать с пришедшим пакетом.
        crow_incoming_handler(pack);
        return;
    }
    else
    {

        //Ветка транзитного пакета. Логика поиска врат и пересылки.
        if (pack->ingate && crow::retransling_allowed == false)
        {
            static int warned = 0;
            if (warned == 0)
            {
                warned = 1;
#ifndef MEMORY_ECONOMY
                crow::warn(
                    "Crow get retransling request but retransling is not "
                    "allowed.\n"
                    "Set crow::retransling_allowed option for enable "
                    "retransling\n"
                    "Or use --retransler option if utility allowed it.\n");
#endif
            }

            system_lock();
            crow::utilize(pack);
            system_unlock();
            return;
        }

        crow_tower_send_to_gate_phase(pack);
    }
}

uint16_t __seqcounter = 0;
crow::packet_ptr crow_transport(crow::packet *pack, bool async)
{
    pack->set_stage(0);
    pack->set_ack(0);
    system_lock();
    pack->set_seqid(__seqcounter++);
    system_unlock();

    crow::packet_ptr ptr(pack);

    if (async)
    {
        return crow::travel(pack);
    }
    else
    {

        crow_do_travel(pack);

        // Делаем unsleep, чтобы перерасчитать таймауты.
        /*        if (crow::unsleep_handler && pack->quality() != 0)
                {
                    crow::unsleep_handler();
                }

         #ifdef CROW_USE_ASYNCIO
                crow::asyncio.unsleep();
        #endif
            }*/
        return ptr;
    }
}

void crow::nocontrol_travel(crow::packet *pack, bool fastsend)
{
    if (fastsend)
    {
        crow_do_travel(pack);
        return;
    }

    system_lock();
    dlist_add_tail(&pack->lnk, &crow_travelled);
    system_unlock();
}

crow::packet_ptr crow::send(const crow::hostaddr_view &addr,
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

    return crow_transport(pack, async);
}

crow::packet_ptr crow::send_v(const crow::hostaddr_view &addr,
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

    return crow_transport(pack, async);
}

crow::packet_ptr crow::send_vv(const crow::hostaddr_view &addr,
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

    return crow_transport(pack, async);
}

crow::packet_ptr crow::send_vvv(const crow::hostaddr_view &addr,
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

    return crow_transport(pack, async);
}

void crow::return_to_tower(crow::packet *pack, uint8_t sts)
{
    pack->u.f.sended_to_gate = 0;
    assert(pack);

    system_lock();

    if (pack->ingate != NULL)
    {
        //Пакет был отправлен, и он нездешний. Уничтожить.
        crow::utilize(pack);
    }
    else
    {
        //Пакет здешний.
        if (sts != CROW_SENDED || pack->quality() == CROW_WITHOUT_ACK)
            crow::tower_release(pack);
        else if (!add_to_outters_list(pack))
        {
            // List full, mark as undelivered
            pack->u.f.undelivered = 1;
            crow::tower_release(pack);
        }
    }

    system_unlock();
}

void crow_undelivered(crow::packet *pack)
{
    pack->u.f.undelivered = 1;

    if (crow::undelivered_handler)
    {
        _in_undelivered_handler = true;
        crow::undelivered_handler(pack);
        _in_undelivered_handler = false;
    }

    if (CROW_NODE_PROTOCOL == pack->type())
    {
        _in_undelivered_handler = true;
        crow::node_protocol.undelivered(pack);
        _in_undelivered_handler = false;
        return;
    }

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    else if (CROW_PUBSUB_PROTOCOL == pack->type())
    {
        _in_undelivered_handler = true;
        crow::pubsub_protocol.undelivered(pack);
        _in_undelivered_handler = false;
        return;
    }
#endif
}

static inline void crow_onestep_send_stage()
{
    crow::packet *pack;

    system_lock();

    while (1)
    {

        if (dlist_empty(&crow_travelled))
        {
            break;
        }

        pack = dlist_first_entry(&crow_travelled, crow::packet, lnk);
        dlist_del_init(&pack->lnk);

        system_unlock();
        crow_do_travel(pack);

        system_lock();
    }
    system_unlock();
}

void crow_onestep_outers_stage()
{
    crow::packet *pack;
    crow::packet *n;

    uint16_t curtime = igris::millis();

    system_lock();
    if (dlist_empty(&crow_outters))
    {
        system_unlock();
        return;
    }

    dlist_for_each_entry_safe(pack, n, &crow_outters, lnk)
    {
        assert(pack->u.f.released_by_tower == 0);

        if (curtime - pack->last_request_time >= pack->ackquant())
        {
            dlist_del_init(&pack->lnk);

            if (pack->_ackcount != 0xFFFF)
                --pack->_ackcount;

            if (pack->_ackcount == 0)
            {
                if (outters_count > 0)
                    --outters_count;
                crow_undelivered(pack);
                crow::tower_release(pack);
            }
            else
            {
                // Retransmit - packet stays in outters (re-added via travel)
                // Don't decrement counter as add_to_outters_list won't increment
                // for retransmits (detected via non-empty hlnk)
                if (__diagnostic_enabled)
                {
                    dpr("OUTFLT: RETRANSMIT seqid=");
                    dprhex(pack->seqid());
                    dpr(" ackcount=");
                    dpr(pack->_ackcount);
                    dln();
                }
                system_unlock();
                crow::travel(pack);
                system_lock();
            }
        }
    }
    system_unlock();
}

void crow_onestep_incoming_stage()
{
    crow::packet *pack;
    crow::packet *n;

    uint16_t curtime = igris::millis();

    system_lock();
    if (dlist_empty(&crow_incoming))
    {
        system_unlock();
        return;
    }

    dlist_for_each_entry_safe(pack, n, &crow_incoming, lnk)
    {
        assert(pack->u.f.released_by_tower == 0);

        if (curtime - pack->last_request_time >= pack->ackquant())
        {
            dlist_del_init(&pack->lnk);

            if (pack->_ackcount != 0xFFFF)
                --pack->_ackcount;

            if (pack->_ackcount == 0)
            {
                // Remove from hash table
                if (__diagnostic_enabled)
                {
                    dpr("DUPFLT: TIMEOUT removing seqid=");
                    dprhex(pack->seqid());
                    dpr(" from htable, inc_count=");
                    dpr(incoming_count);
                    dln();
                }
                crow_incoming_htable.remove(&pack->ihlnk);
                if (incoming_count > 0)
                    --incoming_count;
                pack->u.f.undelivered = 1;
                // Use tower_release instead of utilize to respect
                // released_by_world flag and refs count
                pack->u.f.released_by_tower = true;
                if (pack->u.f.released_by_world && pack->refs == 0)
                    crow::utilize(pack);
            }
            else
            {
                // Re-add to sorted list for retry, don't change counter
                // Hash table entry stays valid
                dlist_move_sorted(pack, &crow_incoming, lnk,
                                  crow_time_comparator);
                pack->last_request_time = curtime;
                system_unlock();
                crow_send_ack(pack);
                system_lock();
            }
        }
    }
    system_unlock();
}

void crow_onestep_keepalive_stage()
{
    crow::keepalive_timer_manager.exec(igris::millis());
}

void crow::onestep()
{
#ifndef CROW_USE_ASYNCIO
    crow::gateway *gate;
    dlist_for_each_entry(gate, &crow::gateway_list, lnk)
    {
        gate->nblock_onestep();
    }
#endif

    crow_onestep_send_stage();
    crow_onestep_keepalive_stage();
    crow_onestep_outers_stage();
    crow_onestep_incoming_stage();
}

int crow::incomming_stage_count()
{
    return dlist_size(&crow_incoming);
}
int crow::outers_stage_count()
{
    return dlist_size(&crow_outters);
}

bool crow::has_untravelled()
{
    system_lock();

    int ret = !(dlist_empty(&crow_travelled) && dlist_empty(&crow_outters) &&
                dlist_empty(&crow_incoming));

    system_unlock();
    return ret;
}

bool crow::has_untravelled_now()
{
    system_lock();

    int ret = !dlist_empty(&crow_travelled);

    system_unlock();
    return ret;
}

void crow::finish()
{
    crow::gateway *gate;
    dlist_for_each_entry(gate, &crow::gateway_list, lnk)
    {
        gate->finish();
    }
}

void crow::reset_for_test()
{
    system_lock();

    // Clear travelled list
    while (!dlist_empty(&crow_travelled))
    {
        crow::packet *pack =
            dlist_first_entry(&crow_travelled, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    // Clear incoming list
    while (!dlist_empty(&crow_incoming))
    {
        crow::packet *pack =
            dlist_first_entry(&crow_incoming, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    // Clear outters list
    while (!dlist_empty(&crow_outters))
    {
        crow::packet *pack =
            dlist_first_entry(&crow_outters, crow::packet, lnk);
        dlist_del(&pack->lnk);
        crow::deallocate_packet(pack);
    }

    crow::total_travelled = 0;
    crow::reset_allocated_count();

    system_unlock();
}

bool crow::fully_empty()
{
    system_lock();
    int ret = dlist_empty(&crow_travelled) && dlist_empty(&crow_incoming) &&
              dlist_empty(&crow_outters);
    system_unlock();
    return ret;
}

int64_t crow::get_minimal_timeout()
{
    // TODO : Ошибки в учёте переходов через uint16_t

    // int64_t result;
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

    if (!dlist_empty(&crow_incoming))
    {
        crow::packet *i = dlist_first_entry(&crow_incoming, crow::packet, lnk);
        update_candidate(i->last_request_time + i->ackquant() - curtime);
    }

    if (!dlist_empty(&crow_outters))
    {
        crow::packet *o = dlist_first_entry(&crow_outters, crow::packet, lnk);
        update_candidate(o->last_request_time + o->ackquant() - curtime);
    }

    if (mininterval == std::numeric_limits<int64_t>::max())
        return -1;

    else if (mininterval < 0)
        return 0;

    else
        return mininterval;
}

void crow::set_time_wait_duration(uint32_t duration_ms)
{
    time_wait_duration_ms = duration_ms;
}

uint32_t crow::get_time_wait_duration()
{
    return time_wait_duration_ms;
}

void crow::set_initial_seqid(uint16_t seqid)
{
    __seqcounter = seqid;
}

uint16_t crow::get_current_seqid()
{
    return __seqcounter;
}
