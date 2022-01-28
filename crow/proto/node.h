#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/keepalive.h>
#include <crow/packet.h>
#include <crow/packet_ptr.h>
#include <crow/proto/protocol.h>
#include <crow/tower.h>

#include <igris/binreader.h>
#include <igris/datastruct/dlist.h>
#include <igris/sync/syslock.h>

#include <stdint.h>

#define CROW_NODEPACK_COMMON 0
#define CROW_NODEPACK_ERROR 1
#define CROW_ERRNO_UNREGISTRED_RID 33

namespace crow
{
#if OLD_HEADER
    using nodeid_t = uint16_t;
#else
    using nodeid_t = uint8_t;
#endif
    class node;

    crow::packet_ptr node_send(nodeid_t sid, nodeid_t rid,
                               const crow::hostaddr_view &addr,
                               const igris::buffer data, uint8_t qos,
                               uint16_t ackquant);

    crow::packet_ptr node_send_v(nodeid_t sid, nodeid_t rid,
                                 const crow::hostaddr_view &addr,
                                 const igris::buffer *vec, size_t veclen,
                                 uint8_t qos, uint16_t ackquant);

    crow::packet_ptr node_send_vv(nodeid_t sid, nodeid_t rid,
                                  const crow::hostaddr_view &addr,
                                  const igris::buffer *vec1, size_t veclen1,
                                  const igris::buffer *vec2, size_t veclen2,
                                  uint8_t qos, uint16_t ackquant);

    struct node_subheader
    {
        nodeid_t sid;
        nodeid_t rid;
        union _u
        {
            uint8_t flags = 0;
            struct _f
            {
                uint8_t reserved : 4;
                uint8_t type : 4;
            } f;
        } u;
    } __attribute__((packed));

    crow::node *find_node(size_t id);
    void __link_node(node *srvs, uint16_t id);
    void bind_node_dynamic(node *srvs);

    static auto node_data(crow::packet *pack)
    {
        return igris::buffer(pack->dataptr() + sizeof(node_subheader),
                             pack->datasize() - sizeof(node_subheader));
    }

    class node_keepalive_timer : public igris::managed_timer_base<
                                     igris::timer_spec<decltype(millis())>>
    {
        void execute() override;
    };

    class node
    {
    public:
        struct dlist_head lnk = DLIST_HEAD_INIT(lnk); // Список нодов.
        struct dlist_head waitlnk =
            DLIST_HEAD_INIT(waitlnk); // Список ожидающих прихода сообщения.
        nodeid_t id = 0;

    public:
        node() = default;
        node(const node &) = delete;
        node(node &&) = delete;

        int waitevent();
        void notify_one(int future);
        void notify_all(int future);

        node &bind(int addr)
        {
            system_lock();
            __link_node(this, addr);
            system_unlock();
            return *this;
        };

        node &bind()
        {
            bind_node_dynamic(this);
            return *this;
        };

        crow::packet_ptr send(nodeid_t rid, const crow::hostaddr_view &raddr,
                              const igris::buffer data,
                              uint8_t qos = CROW_DEFAULT_QOS,
                              uint16_t ackquant = CROW_DEFAULT_ACKQUANT)
        {
            if (id == 0)
                bind();
            return crow::node_send(id, rid, raddr, data, qos, ackquant);
        }

        crow::packet_ptr send_v(nodeid_t rid, const crow::hostaddr_view &raddr,
                                const igris::buffer *vdat, size_t vlen,
                                uint8_t qos, uint16_t ackquant)
        {
            if (id == 0)
                bind();
            return crow::node_send_v(id, rid, raddr, vdat, vlen, qos, ackquant);
        }

        crow::packet_ptr send_vv(nodeid_t rid, const crow::hostaddr_view &raddr,
                                 const igris::buffer *vdat1, size_t vlen1,
                                 const igris::buffer *vdat2, size_t vlen2,
                                 uint8_t qos, uint16_t ackquant)
        {
            if (id == 0)
                bind();
            return crow::node_send_vv(id, rid, raddr, vdat1, vlen1, vdat2,
                                      vlen2, qos, ackquant);
        }

        static crow::node_subheader *subheader(crow::packet *pack)
        {
            return (crow::node_subheader *)pack->dataptr();
        }

        virtual ~node();

    private:
        virtual void incoming_packet(crow::packet *pack) = 0;

        virtual void undelivered_packet(crow::packet *pack)
        {
            notify_all(-1);
            crow::release(pack);
        }

        friend class node_protocol_cls;
    };

    class alived_object
    {
    public:
        node_keepalive_timer keepalive_timer;

        virtual void keepalive_handle() {}

        void install_keepalive(int64_t interval, bool immediate_call=true)
        {
            crow::keepalive_timer_manager.plan(
                (igris::managed_timer_base<
                    igris::timer_spec<decltype(millis())>> &)keepalive_timer,
                millis(), interval);

            crow::unsleep_handler();

            if (immediate_call) 
            {
                keepalive_handle();
            }
        };

        virtual ~alived_object();
    };

    class node_protocol_cls : public crow::protocol
    {
    private:
        void send_node_error(crow::packet *pack, int errcode);

    public:
        void incoming(crow::packet *pack);
        void undelivered(crow::packet *pack);

        node_protocol_cls() /*: protocol(CROW_NODE_PROTOCOL)*/ {}

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
    extern struct dlist_head nodes_list;

    class node_packet_ptr : public packet_ptr
    {
        node *_node;

    public:
        node_packet_ptr(crow::packet *pack_, node *node_)
            : packet_ptr(pack_), _node(node_)
        {
        }
        node_packet_ptr(const crow::packet_ptr &oth, node *node_)
            : packet_ptr(oth), _node(node_)
        {
        }
        node_packet_ptr(crow::packet_ptr &&oth, node *node_)
            : packet_ptr(std::move(oth)), _node(node_)
        {
        }
        node_packet_ptr(std::nullptr_t) : packet_ptr(nullptr), _node(nullptr) {}

        int rid()
        {
            auto *h = node::subheader(pack);
            return h->rid;
        }

        int sid()
        {
            auto *h = node::subheader(pack);
            return h->sid;
        }

        igris::buffer message() { return node_data(pack); }

        void reply(igris::buffer rep)
        {
            _node->send(sid(), pack->addr(), rep, pack->quality(),
                        pack->ackquant());
        }
    };
} // namespace crow

#endif
