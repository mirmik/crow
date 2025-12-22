#include <crow/proto/node.h>
#include <crow/proto/node_protocol.h>
#include <crow/tower_cls.h>
#include <crow/warn.h>
#include <igris/time/systime.h>
#include <nos/fprint.h>
#include <vector>

// ============================================================================
// reassembly_buffer implementation
// ============================================================================

void crow::reassembly_buffer::release_all()
{
    for (auto &pair : chunks)
    {
        if (pair.second)
        {
            crow::Tower::release(pair.second);
        }
    }
    chunks.clear();
    expected_chunks = 0;
    total_payload_size = 0;
}

crow::packet *crow::reassembly_buffer::assemble(Tower &tower)
{
    if (!is_complete())
        return nullptr;

    // Calculate total data size
    size_t total_size = 0;
    for (auto &pair : chunks)
    {
        total_size += node_data(pair.second).size();
    }

    // Get first chunk for header info
    auto *first_chunk = chunks[0];
    auto &first_sh = first_chunk->subheader<node_subheader>();

    // Create new packet with assembled data
    // Use same address as first chunk
    crow::packet *assembled = crow::allocate_packet<crow::header_v1>(
        first_chunk->addrsize(),
        sizeof(node_subheader) + total_size);

    if (!assembled)
    {
        crow::warn("Failed to allocate assembled packet");
        return nullptr;
    }

    // Copy address from first chunk
    memcpy(assembled->addrptr(), first_chunk->addrptr(), first_chunk->addrsize());

    // Copy header from first chunk (without chunked flags)
    auto &new_sh = assembled->subheader<node_subheader>();
    new_sh.sid = first_sh.sid;
    new_sh.rid = first_sh.rid;
    new_sh.u.f.type = first_sh.u.f.type;
    new_sh.u.f.chunked = 0;
    new_sh.u.f.has_more = 0;

    // Copy QoS and other settings from first chunk
    assembled->set_quality(first_chunk->quality());
    assembled->set_ackquant(first_chunk->ackquant());
    assembled->ingate = first_chunk->ingate;

    // Assemble payload in order
    char *dest = assembled->dataptr() + sizeof(node_subheader);
    for (uint16_t i = 0; i < expected_chunks; ++i)
    {
        auto payload = node_data(chunks[i]);
        memcpy(dest, payload.data(), payload.size());
        dest += payload.size();
    }

    // Release all chunk packets
    release_all();

    return assembled;
}

// ============================================================================
// node_protocol_cls implementation
// ============================================================================

uint64_t crow::node_protocol_cls::hash_address(const uint8_t *addr, size_t len)
{
    // Simple FNV-1a hash
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i)
    {
        hash ^= addr[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

void crow::node_protocol_cls::cleanup_stale_buffers(uint64_t now_ms)
{
    // Collect keys to erase to avoid iterator ABI issues
    std::vector<reassembly_key> to_erase;

    for (auto &pair : _reassembly)
    {
        if (now_ms - pair.second.start_time_ms > _reassembly_timeout_ms)
        {
            nos::fprintln("node_protocol: reassembly timeout for sid={} rid={}",
                          pair.first.sid, pair.first.rid);
            pair.second.release_all();
            to_erase.push_back(pair.first);
        }
    }

    for (const auto &key : to_erase)
    {
        _reassembly.erase(key);
    }
}

void crow::node_protocol_cls::handle_chunk(crow::packet *pack, Tower &tower)
{
    auto &sh = pack->subheader<node_subheader>();
    auto *chunk_hdr = (node_chunk_header *)(pack->dataptr() + sizeof(node_subheader));

    uint16_t chunk_id = chunk_hdr->chunk_id;
    bool has_more = sh.u.f.has_more;

    nos::fprintln("node_protocol: chunk received sid={} rid={} chunk_id={} has_more={} size={}",
                  sh.sid, sh.rid, chunk_id, has_more, pack->datasize());

    // Validate chunk_id
    if (chunk_id >= NODE_MAX_CHUNKS)
    {
        nos::fprintln("node_protocol: chunk_id {} exceeds limit", chunk_id);
        Tower::release(pack);
        return;
    }

    // Build reassembly key
    reassembly_key key;
    key.rid = sh.rid;
    key.sid = sh.sid;
    key.addr_hash = hash_address(pack->addrptr(), pack->addrsize());

    uint64_t now_ms = igris::millis();

    // Cleanup stale buffers periodically
    cleanup_stale_buffers(now_ms);

    // Get or create reassembly buffer
    auto &buffer = _reassembly[key];
    if (buffer.chunks.empty())
    {
        buffer.start_time_ms = now_ms;
    }

    // Check for duplicate chunk
    if (buffer.chunks.count(chunk_id))
    {
        // Duplicate - release old, keep new (in case of retransmit)
        Tower::release(buffer.chunks[chunk_id]);
    }

    // Store chunk
    buffer.chunks[chunk_id] = pack;
    buffer.total_payload_size += node_data(pack).size();

    // Check max message size
    if (buffer.total_payload_size > _max_message_size)
    {
        nos::fprintln("node_protocol: message size {} exceeds limit {}",
                      buffer.total_payload_size, _max_message_size);
        buffer.release_all();
        _reassembly.erase(key);
        return;
    }

    // If this is the last chunk, we now know total count
    if (!has_more)
    {
        buffer.expected_chunks = chunk_id + 1;
    }

    // Try to assemble
    if (buffer.is_complete())
    {
        nos::fprintln("node_protocol: assembling {} chunks, total_size={}",
                      buffer.expected_chunks, buffer.total_payload_size);
        crow::packet *assembled = buffer.assemble(tower);
        _reassembly.erase(key);

        if (assembled)
        {
            nos::fprintln("node_protocol: assembled packet size={}, delivering to node",
                          assembled->datasize());
            // Deliver assembled packet
            deliver_to_node(assembled, tower);
        }
        else
        {
            nos::fprintln("node_protocol: assembly failed!");
        }
    }
}

void crow::node_protocol_cls::deliver_to_node(crow::packet *pack, Tower &tower)
{
    auto &sh = pack->subheader<node_subheader>();
    crow::node *srv = nullptr;

    for (crow::node &srvs : crow::nodes_list)
    {
        if (srvs.id == sh.rid)
        {
            srv = &srvs;
            break;
        }
    }

    if (srv == nullptr)
    {
        send_node_error(pack, CROW_ERRNO_UNREGISTRED_RID, tower);
        Tower::release(pack);
        return;
    }

    switch (sh.u.f.type)
    {
        case CROW_NODEPACK_COMMON:
            srv->incoming_packet(pack);
            break;

        case CROW_NODEPACK_ERROR:
            srv->notify_one(get_error_code(pack));
            Tower::release(pack);
            break;
    }
}

void crow::node_protocol_cls::send_node_error(crow::packet *pack, int errcode,
                                              Tower &tower)
{
    crow::node_subheader sh;

    sh.sid = crow::node_protocol.rid(pack);
    sh.rid = crow::node_protocol.sid(pack);
    sh.u.f.type = CROW_NODEPACK_ERROR;
    sh.u.f.chunked = 0;
    sh.u.f.has_more = 0;

    const nos::buffer iov[2] = {{(char *)&sh, sizeof(sh)},
                                {(char *)&errcode, sizeof(errcode)}};

    tower.send_v({pack->addrptr(), pack->addrsize()}, iov, 2,
                 CROW_NODE_PROTOCOL, 0, pack->ackquant(), true);
}

void crow::node_protocol_cls::incoming(crow::packet *pack, Tower &tower)
{
    auto &sh = pack->subheader<node_subheader>();

    // Handle chunked messages
    if (sh.u.f.chunked)
    {
        handle_chunk(pack, tower);
        return;
    }

    // Non-chunked message - deliver directly
    deliver_to_node(pack, tower);
}

void crow::node_protocol_cls::undelivered(crow::packet *pack, Tower &tower)
{
    (void)tower;
    crow::node_subheader *sh = (crow::node_subheader *)pack->dataptr();
    for (crow::node &srvs : crow::nodes_list)
    {
        if (srvs.id == sh->sid)
        {
            srvs.undelivered_packet(pack);
            return;
        }
    }
    Tower::release(pack);
}

void crow::node_protocol_cls::delivered(crow::packet *pack, Tower &tower)
{
    (void)tower;
    crow::node_subheader *sh = (crow::node_subheader *)pack->dataptr();

    for (auto &srvs : crow::nodes_list)
    {
        if (srvs.id == sh->sid)
        {
            srvs.delivered_packet(pack);
            return;
        }
    }

    Tower::release(pack);
}
