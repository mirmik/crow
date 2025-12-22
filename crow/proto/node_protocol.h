#ifndef CROW_NODE_PROTOCOL_H
#define CROW_NODE_PROTOCOL_H

#include <crow/proto/protocol.h>
#include <igris/container/dlist.h>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace crow
{
    class Tower; // Forward declaration
    using nodeid_t = uint16_t;

    struct node_subheader
    {
        nodeid_t sid = 0;
        nodeid_t rid = 0;
        union _u
        {
            uint8_t flags;
            struct _f
            {
                uint8_t has_more : 1;  // more chunks follow
                uint8_t chunked : 1;   // this is a chunked message
                uint8_t reserved : 2;
                uint8_t type : 4;
            } f;
        } u = {};
    } __attribute__((packed));

    // Chunk header follows node_subheader when chunked=1
    // Format: [chunk_id:2 bytes LE]
    struct node_chunk_header
    {
        uint16_t chunk_id = 0;
    } __attribute__((packed));

    // Constants for chunked protocol
    inline constexpr size_t NODE_CHUNK_HEADER_SIZE = sizeof(node_chunk_header);
    inline constexpr size_t NODE_DEFAULT_MAX_MESSAGE_SIZE = 64 * 1024;  // 64 KiB
    inline constexpr uint32_t NODE_DEFAULT_REASSEMBLY_TIMEOUT_MS = 5000;  // 5 seconds
    inline constexpr uint16_t NODE_MAX_CHUNKS = 1024;

    // Get data portion of a node packet (excluding headers)
    static inline nos::buffer node_data(crow::packet *pack)
    {
        auto *sh = (node_subheader *)pack->dataptr();
        size_t header_size = sizeof(node_subheader);
        if (sh->u.f.chunked)
        {
            header_size += NODE_CHUNK_HEADER_SIZE;
        }
        return nos::buffer(pack->dataptr() + header_size,
                           pack->datasize() - header_size);
    }

    // Key for identifying a reassembly session
    struct reassembly_key
    {
        nodeid_t rid;        // destination node
        nodeid_t sid;        // source node
        uint64_t addr_hash;  // hash of source address

        bool operator==(const reassembly_key &o) const
        {
            return rid == o.rid && sid == o.sid && addr_hash == o.addr_hash;
        }
    };
}

// Hash specialization for reassembly_key
namespace std
{
    template <>
    struct hash<crow::reassembly_key>
    {
        size_t operator()(const crow::reassembly_key &k) const
        {
            // FNV-1a constants depend on platform word size
#if SIZE_MAX == 0xFFFFFFFFFFFFFFFFULL
            // 64-bit platform
            constexpr size_t fnv_offset = 14695981039346656037ULL;
            constexpr size_t fnv_prime = 1099511628211ULL;
#else
            // 32-bit platform
            constexpr size_t fnv_offset = 2166136261U;
            constexpr size_t fnv_prime = 16777619U;
#endif
            size_t h = fnv_offset;
            h ^= k.rid;
            h *= fnv_prime;
            h ^= k.sid;
            h *= fnv_prime;
            h ^= static_cast<size_t>(k.addr_hash);
            h *= fnv_prime;
            return h;
        }
    };
}

namespace crow
{

    // Buffer for reassembling chunked messages
    struct reassembly_buffer
    {
        std::unordered_map<uint16_t, crow::packet *> chunks;
        uint16_t expected_chunks = 0;  // 0 = unknown (last chunk not yet received)
        uint64_t start_time_ms = 0;
        size_t total_payload_size = 0;

        bool is_complete() const
        {
            if (expected_chunks == 0)
                return false;
            return chunks.size() == expected_chunks;
        }

        // Assemble all chunks into a new packet
        // Returns nullptr on failure
        crow::packet *assemble(Tower &tower);

        // Release all held packets
        void release_all();
    };

    class node_protocol_cls : public crow::protocol
    {
    private:
        std::unordered_map<reassembly_key, reassembly_buffer> _reassembly;
        size_t _max_message_size = NODE_DEFAULT_MAX_MESSAGE_SIZE;
        uint32_t _reassembly_timeout_ms = NODE_DEFAULT_REASSEMBLY_TIMEOUT_MS;
        bool _debug = false;

        void send_node_error(crow::packet *pack, int errcode, Tower &tower);
        void deliver_to_node(crow::packet *pack, Tower &tower);
        void handle_chunk(crow::packet *pack, Tower &tower);
        void cleanup_stale_buffers(uint64_t now_ms);

        static uint64_t hash_address(const uint8_t *addr, size_t len);

    public:
        void incoming(crow::packet *pack, Tower &tower) override;
        void undelivered(crow::packet *pack, Tower &tower) override;
        void delivered(crow::packet *pack, Tower &tower);

        node_protocol_cls() {}

        // Configuration
        void set_max_message_size(size_t size) { _max_message_size = size; }
        size_t max_message_size() const { return _max_message_size; }

        void set_reassembly_timeout(uint32_t ms) { _reassembly_timeout_ms = ms; }
        uint32_t reassembly_timeout() const { return _reassembly_timeout_ms; }

        void set_debug(bool debug) { _debug = debug; }
        bool debug() const { return _debug; }

        // Statistics
        size_t reassembly_sessions_count() const { return _reassembly.size(); }

        static auto sid(crow::packet *pack)
        {
            return ((node_subheader *)(pack->dataptr()))->sid;
        }
        static auto rid(crow::packet *pack)
        {
            return ((node_subheader *)(pack->dataptr()))->rid;
        }

        static auto get_error_code(crow::packet *pack)
        {
            return *(int *)(node_data(pack).data());
        }
    };
    extern node_protocol_cls node_protocol;
}

#endif