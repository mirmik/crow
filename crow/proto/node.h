#ifndef CROW_NODE_H
#define CROW_NODE_H

#include <crow/packet.h>
#include <crow/proto/protocol.h>
#include <crow/types.h>

#include <igris/binreader.h>
#include <igris/datastruct/dlist.h>
#include <igris/sync/syslock.h>

#define CROW_NODEPACK_COMMON 0
#define CROW_NODEPACK_ERROR 1

#define CROW_NODE_SPECIAL_BUS_ERROR -2
#define CROW_ERRNO_UNREGISTRED_RID 33

namespace crow
{
    using nodeid_t = uint8_t;
    class node;

    crow::packet_ptr node_send(nodeid_t sid, nodeid_t rid,
                               const crow::hostaddr_view &addr,
                               const igris::buffer data, uint8_t qos,
                               uint16_t ackquant);

    crow::packet_ptr node_send_special(nodeid_t sid, nodeid_t rid,
                                       const crow::hostaddr_view &addr,
                                       uint8_t type, const igris::buffer data,
                                       uint8_t qos, uint16_t ackquant);

    crow::packet_ptr node_send_v(nodeid_t sid, nodeid_t rid,
                                 const crow::hostaddr_view &addr,
                                 const igris::buffer *vec, size_t veclen,
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

    static auto node_data(crow_packet *pack)
    {
        return igris::buffer(crow_packet_dataptr(pack) + sizeof(node_subheader),
                             crow_packet_datasize(pack) -
                                 sizeof(node_subheader));
    }

    class node
    {
      public:
        struct dlist_head lnk = DLIST_HEAD_INIT(lnk); // Список нодов.
        struct dlist_head waitlnk =
            DLIST_HEAD_INIT(waitlnk); // Список ожидающих прихода сообщения.
        nodeid_t id = 0;

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

        crow::packet_ptr send_special(nodeid_t rid,
                                      const crow::hostaddr_view &raddr,
                                      uint8_t type, const igris::buffer data,
                                      uint8_t qos = CROW_DEFAULT_QOS,
                                      uint16_t ackquant = CROW_DEFAULT_ACKQUANT)
        {
            if (id == 0)
                bind();
            return crow::node_send_special(id, rid, raddr, type, data, qos,
                                           ackquant);
        }

        crow::packet_ptr send_v(nodeid_t rid, const crow::hostaddr_view &raddr,
                                const igris::buffer *vdat, size_t vlen,
                                uint8_t qos, uint16_t ackquant)
        {
            if (id == 0)
                bind();
            return crow::node_send_v(id, rid, raddr, vdat, vlen, qos, ackquant);
        }

        static node_subheader *subheader(crow_packet *pack)
        {
            return (crow::node_subheader *)crow_packet_dataptr(pack);
        }

        virtual ~node();

      private:
        virtual void incoming_packet(crow_packet *pack) = 0;

        virtual void undelivered_packet(crow_packet *pack)
        {
            notify_all(-1);
            crow::release(pack);
        }

        friend class node_protocol_cls;
    };

    class node_protocol_cls : public crow::protocol
    {
      private:
        void send_node_error(crow_packet *pack, int errcode);

      public:
        void incoming(crow_packet *pack) override;
        void undelivered(crow_packet *pack) override;

        node_protocol_cls() : protocol(CROW_NODE_PROTOCOL) {}

        static auto sid(crow_packet *pack)
        {
            return ((node_subheader *)(crow_packet_dataptr(pack)))->sid;
        }
        static auto rid(crow_packet *pack)
        {
            return ((node_subheader *)(crow_packet_dataptr(pack)))->rid;
        }

        static auto get_error_code(crow_packet *pack)
        {
            return *(int *)(node_data(pack).data());
        }
    };
    extern node_protocol_cls node_protocol;
    extern struct dlist_head nodes_list;

    class node_packet_ptr : public packet_ptr
    {
      public:
        node_packet_ptr(crow_packet *pack_) : packet_ptr(pack_) {}
        node_packet_ptr(const crow::packet_ptr &oth) : packet_ptr(oth) {}
        node_packet_ptr(crow::packet_ptr &&oth) : packet_ptr(std::move(oth)) {}

        int rid()
        {
            auto *h = node::subheader(pack);
            return h->rid;
        }

        igris::buffer message() { return node_data(pack); }
    };
} // namespace crow

#endif
