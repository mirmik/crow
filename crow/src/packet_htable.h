#ifndef CROW_PACKET_HTABLE_H
#define CROW_PACKET_HTABLE_H

/**
 * Simple intrusive hash table for crow packets.
 * Key: (seqid, addr) pair
 * Uses chaining with dlist for collision resolution.
 */

#include <igris/datastruct/dlist.h>
#include <cstring>
#include <cstdint>

namespace crow
{
    class packet;

    // Default bucket count - power of 2 for fast modulo
    inline constexpr size_t PACKET_HTABLE_BUCKETS = 64;

    /**
     * Simple hash table for fast packet lookup by seqid+addr.
     * Uses separate chaining - each bucket is a dlist.
     * Packets must have a dlist_head member for hash table linkage.
     */
    class packet_htable
    {
        dlist_head _buckets[PACKET_HTABLE_BUCKETS];
        size_t _count = 0;

    public:
        packet_htable()
        {
            for (size_t i = 0; i < PACKET_HTABLE_BUCKETS; ++i)
            {
                dlist_init(&_buckets[i]);
            }
        }

        /// Compute bucket index from seqid
        static size_t bucket_index(uint16_t seqid)
        {
            // Simple hash - seqid modulo bucket count
            // Since PACKET_HTABLE_BUCKETS is power of 2, use bitwise AND
            return seqid & (PACKET_HTABLE_BUCKETS - 1);
        }

        /// Add packet to hash table using its hlnk member
        void put(packet *pack, dlist_head *hlnk, uint16_t seqid)
        {
            size_t idx = bucket_index(seqid);
            dlist_add_tail(hlnk, &_buckets[idx]);
            ++_count;
        }

        /// Remove packet from hash table
        void remove(dlist_head *hlnk)
        {
            dlist_del_init(hlnk);
            --_count;
        }

        /// Get bucket head for iteration
        dlist_head *bucket(uint16_t seqid)
        {
            return &_buckets[bucket_index(seqid)];
        }

        /// Number of packets in table
        size_t count() const
        {
            return _count;
        }

        /// Check if empty
        bool empty() const
        {
            return _count == 0;
        }
    };
}

#endif
