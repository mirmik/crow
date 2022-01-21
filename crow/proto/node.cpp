#include <crow/defs.h>
#include <crow/proto/node.h>
#include <crow/tower.h>

#include <crow/print.h>
#include <igris/sync/syslock.h>
#include <igris/util/numconvert.h>

#include <nos/print.h>

DLIST_HEAD(crow::nodes_list);
crow::node_protocol_cls crow::node_protocol;

crow::packet_ptr crow::node_send(nodeid_t sid, nodeid_t rid,
                                 const crow::hostaddr_view &addr,
                                 const igris::buffer data, uint8_t qos,
                                 uint16_t ackquant)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const igris::buffer iov[2] = {{(char *)&sh, sizeof(sh)},
                                  {(char *)data.data(), data.size()}};

    return crow::send_v(addr, iov, 2, CROW_NODE_PROTOCOL, qos, ackquant);
}

crow::packet_ptr crow::node_send_v(nodeid_t sid, nodeid_t rid,
                                   const crow::hostaddr_view &addr,
                                   const igris::buffer *vec, size_t veclen,
                                   uint8_t qos, uint16_t ackquant)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const igris::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return crow::send_vv(addr, iov, 1, vec, veclen, CROW_NODE_PROTOCOL, qos,
                         ackquant);
}

crow::packet_ptr crow::node_send_vv(nodeid_t sid, nodeid_t rid,
                                   const crow::hostaddr_view &addr,
                                   const igris::buffer *vec1, size_t veclen1,
                                   const igris::buffer *vec2, size_t veclen2,
                                   uint8_t qos, uint16_t ackquant)
{
    crow::node_subheader sh;
    sh.sid = sid;
    sh.rid = rid;
    sh.u.f.type = CROW_NODEPACK_COMMON;

    const igris::buffer iov[1] = {{(char *)&sh, sizeof(sh)}};

    return crow::send_vvv(addr, iov, 1, vec1, veclen1, vec2, veclen2, 
        CROW_NODE_PROTOCOL, qos, ackquant);
}

void crow::node_protocol_cls::send_node_error(crow::packet *pack,
                                              int errcode)
{
    crow::node_subheader sh;

    sh.sid = crow::node_protocol.rid(pack);
    sh.rid = crow::node_protocol.sid(pack);
    sh.u.f.type = CROW_NODEPACK_ERROR;

    const igris::buffer iov[2] = {{(char *)&sh, sizeof(sh)},
                                  {(char *)&errcode, sizeof(errcode)}};

    crow::send_v({pack->addrptr(), pack->addrsize()}, iov,
                 2, CROW_NODE_PROTOCOL, 0, pack->ackquant());
}

void crow::node_protocol_cls::incoming(crow::packet *pack)
{
    auto &sh = pack->subheader<node_subheader>();
    crow::node *srv = nullptr;

    crow::node *srvs;
    dlist_for_each_entry(srvs, &crow::nodes_list, lnk)
    {
        if (srvs->id == sh.rid)
        {
            srv = srvs;
            break;
        }
    }

    if (srv == nullptr)
    {
        send_node_error(pack, CROW_ERRNO_UNREGISTRED_RID);
        crow::release(pack);
        return;
    }

    switch (sh.u.f.type)
    {
    case CROW_NODEPACK_COMMON:
        srv->incoming_packet(pack);
        break;

    case CROW_NODEPACK_ERROR:
        srv->notify_one(get_error_code(pack));
        crow::release(pack);
        break;
    }
    return;
}

void crow::node_protocol_cls::undelivered(crow::packet *pack)
{
    crow::node_subheader *sh =
        (crow::node_subheader *)pack->dataptr();

    crow::node *srvs;
    dlist_for_each_entry(srvs, &crow::nodes_list, lnk)
    {
        if (srvs->id == sh->sid)
        {
            srvs->undelivered_packet(pack);
            return;
        }
    }

    crow::release(pack);
}

void crow::__link_node(crow::node *srv, uint16_t id)
{
    srv->id = id;
    dlist_add_tail(&srv->lnk, &nodes_list);
}

crow::node *crow::find_node(size_t id)
{
    // TODO: переделать на хештаблицу
    crow::node *node;
    dlist_for_each_entry(node, &nodes_list, lnk)
    {
        if (node->id == id)
            return node;
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
    if (!dlist_empty(&waitlnk))
        notify_all(-1);
    dlist_del(&lnk);
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
    alive_node& n = *mcast_out(this, alived_object, keepalive_timer);
    n.keepalive_handle();
}