#include <crow/defs.h>
#include <crow/proto/node.h>
#include <crow/tower_cls.h>

#include <crow/print.h>
#include <igris/sync/syslock.h>
#include <igris/util/numconvert.h>

#include <nos/print.h>

igris::dlist<crow::node, &crow::node::lnk> crow::nodes_list;
crow::node_protocol_cls crow::node_protocol;

// Instance methods - use _tower
crow::packet_ptr crow::node::send(nodeid_t rid,
                                  const crow::hostaddr_view &raddr,
                                  const nos::buffer data,
                                  uint8_t qos,
                                  uint16_t ackquant,
                                  bool async)
{
    assert(_tower != nullptr && "Node must be bound to a tower before sending");
    if (id == 0)
        bind_node_dynamic(this);

    crow::node_subheader sh;
    sh.sid = id;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[2] = {{(char *)&sh, sizeof(sh)},
                                {(char *)data.data(), data.size()}};

    return _tower->send_v(raddr, iov, 2, CROW_NODE_PROTOCOL, qos, ackquant,
                          async);
}

crow::packet_ptr crow::node::send_v(nodeid_t rid,
                                    const crow::hostaddr_view &raddr,
                                    const nos::buffer *vdat,
                                    size_t vlen,
                                    uint8_t qos,
                                    uint16_t ackquant,
                                    bool async)
{
    assert(_tower != nullptr && "Node must be bound to a tower before sending");
    if (id == 0)
        bind_node_dynamic(this);

    crow::node_subheader sh;
    sh.sid = id;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return _tower->send_vv(raddr, iov, 1, vdat, vlen, CROW_NODE_PROTOCOL, qos,
                           ackquant, async);
}

crow::packet_ptr crow::node::send_vv(nodeid_t rid,
                                     const crow::hostaddr_view &raddr,
                                     const nos::buffer *vdat1,
                                     size_t vlen1,
                                     const nos::buffer *vdat2,
                                     size_t vlen2,
                                     uint8_t qos,
                                     uint16_t ackquant,
                                     bool async)
{
    assert(_tower != nullptr && "Node must be bound to a tower before sending");
    if (id == 0)
        bind_node_dynamic(this);

    crow::node_subheader sh;
    sh.sid = id;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return _tower->send_vvv(raddr, iov, 1, vdat1, vlen1, vdat2, vlen2,
                            CROW_NODE_PROTOCOL, qos, ackquant, async);
}

// New API with explicit Tower
crow::packet_ptr crow::node::send(Tower &tower,
                                  nodeid_t sid,
                                  nodeid_t rid,
                                  const crow::hostaddr_view &addr,
                                  const nos::buffer data,
                                  uint8_t qos,
                                  uint16_t ackquant,
                                  bool async)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[2] = {{(char *)&sh, sizeof(sh)},
                                {(char *)data.data(), data.size()}};

    return tower.send_v(addr, iov, 2, CROW_NODE_PROTOCOL, qos, ackquant, async);
}

crow::packet_ptr crow::node::send_v(Tower &tower,
                                    nodeid_t sid,
                                    nodeid_t rid,
                                    const crow::hostaddr_view &addr,
                                    const nos::buffer *vec,
                                    size_t veclen,
                                    uint8_t qos,
                                    uint16_t ackquant,
                                    bool async)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return tower.send_vv(addr, iov, 1, vec, veclen, CROW_NODE_PROTOCOL, qos,
                         ackquant, async);
}

crow::packet_ptr crow::node::send_vv(Tower &tower,
                                     nodeid_t sid,
                                     nodeid_t rid,
                                     const crow::hostaddr_view &addr,
                                     const nos::buffer *vec1,
                                     size_t veclen1,
                                     const nos::buffer *vec2,
                                     size_t veclen2,
                                     uint8_t qos,
                                     uint16_t ackquant,
                                     bool async)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const nos::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return tower.send_vvv(addr, iov, 1, vec1, veclen1, vec2, veclen2,
                          CROW_NODE_PROTOCOL, qos, ackquant, async);
}

void crow::__link_node(crow::node *srv, uint16_t id)
{
    srv->id = id;
    nodes_list.move_back(*srv);
}

crow::node *crow::find_node(size_t id)
{
    // TODO: переделать на хештаблицу
    for (auto &node : nodes_list)
    {
        if (node.id == id)
            return &node;
    }

    return nullptr;
}

void crow::bind_node_dynamic(crow::node *srv)
{
    // Динамические порты располагаются в верхнем полупространстве.
    static nodeid_t counter = 1 << (sizeof(nodeid_t) * 8 - 1);

    system_lock();
    do
    {
        counter++;
        if (counter == 0)
            counter = (1 << (sizeof(nodeid_t) * 8 - 1));
    } while (crow::find_node(counter) != nullptr);

    __link_node(srv, counter);
    system_unlock();
}

crow::node::~node()
{
    system_lock();
    if (!waitlnk.empty())
        notify_all(-1);
    lnk.unlink();
    system_unlock();
}

crow::alived_object::~alived_object()
{
    system_lock();
    keepalive_timer.unplan();
    system_unlock();
}

void crow::node_keepalive_timer::execute()
{
    alived_object &n = *mcast_out(this, alived_object, keepalive_timer);
    n.keepalive_handle();
}

// ============================================================================
// Chunked sending implementation
// ============================================================================

void crow::node::send_single_chunk(nodeid_t rid,
                                   const crow::hostaddr_view &raddr,
                                   const char *data,
                                   size_t size,
                                   uint16_t chunk_id,
                                   bool has_more,
                                   uint8_t qos,
                                   uint16_t ackquant)
{
    assert(_tower != nullptr && "Node must be bound to a tower before sending");
    if (id == 0)
        bind_node_dynamic(this);

    crow::node_subheader sh;
    sh.sid = id;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;
    sh.u.f.chunked = 1;
    sh.u.f.has_more = has_more ? 1 : 0;

    crow::node_chunk_header ch;
    ch.chunk_id = chunk_id;

    const nos::buffer iov[3] = {
        {(char *)&sh, sizeof(sh)},
        {(char *)&ch, sizeof(ch)},
        {data, size}
    };

    _tower->send_v(raddr, iov, 3, CROW_NODE_PROTOCOL, qos, ackquant, false);
}

void crow::node::send_chunked(nodeid_t rid,
                              const crow::hostaddr_view &raddr,
                              const nos::buffer data,
                              uint8_t qos,
                              uint16_t ackquant)
{
    // If chunking disabled or data fits in one chunk, send as single packet
    if (_chunk_size == 0 || data.size() <= _chunk_size)
    {
        send(rid, raddr, data, qos, ackquant, false);
        return;
    }

    // Calculate payload per chunk (chunk_size includes headers)
    size_t header_overhead = sizeof(node_subheader) + sizeof(node_chunk_header);
    if (_chunk_size <= header_overhead)
    {
        // Chunk size too small, send as single packet
        send(rid, raddr, data, qos, ackquant, false);
        return;
    }
    size_t payload_per_chunk = _chunk_size - header_overhead;

    // Split and send chunks
    size_t offset = 0;
    uint16_t chunk_id = 0;

    while (offset < data.size())
    {
        size_t remaining = data.size() - offset;
        size_t chunk_payload = (remaining > payload_per_chunk) ? payload_per_chunk : remaining;
        bool has_more = (offset + chunk_payload < data.size());

        send_single_chunk(rid, raddr, data.data() + offset, chunk_payload,
                          chunk_id, has_more, qos, ackquant);

        offset += chunk_payload;
        chunk_id++;
    }
}