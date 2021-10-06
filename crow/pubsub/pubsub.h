/** @file */

#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <assert.h>
#include <crow/proto/protocol.h>
#include <crow/tower.h>

#include <igris/buffer.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>

#include <igris/buffer.h>

struct crow_theme;

#define SUBSCRIBE 0
#define PUBLISH 1
#define MESSAGE 2
#define SERVICE_ANSWER 3

namespace crow
{
    struct subheader_pubsub
    {
        uint8_t type;
        uint8_t thmsz;
    } __attribute__((packed));

    struct subheader_pubsub_data
    {
        uint16_t datsz;
    } __attribute__((packed));

    struct subheader_pubsub_control
    {
        uint8_t qos;
        uint16_t ackquant;
    } __attribute__((packed));

    class pubsub_protocol_cls : public crow::protocol
    {
      public:
        struct dlist_head subscribers = DLIST_HEAD_INIT(subscribers);
        void (*incoming_handler)(crow_packet *);
        void (*undelivered_handler)(crow_packet *);

      public:
        pubsub_protocol_cls() : protocol(CROW_PUBSUB_PROTOCOL) {}

        void incoming(crow_packet *pack) override;
        void undelivered(crow_packet *pack) override;

        static void start_resubscribe_thread(int millis);
        void resubscribe_all();

        static void enable_crowker_subsystem();
        static pubsub_protocol_cls &instance();
    };
    extern pubsub_protocol_cls pubsub_protocol;

    crow::packet_ptr publish(const crow::hostaddr_view &addr,
                             const igris::buffer theme,
                             const igris::buffer data, uint8_t qos,
                             uint16_t acktime, uint8_t type = PUBLISH);

    crow::packet_ptr publish_v(const crow::hostaddr_view &addr,
                               const igris::buffer theme,
                               const igris::buffer *vec, int vecsz, uint8_t qos,
                               uint16_t acktime);

    void subscribe(const crow::hostaddr_view &addr, igris::buffer theme,
                   uint8_t qo0, uint16_t acktime, uint8_t rqos,
                   uint16_t racktime);

    static inline char *packet_pubsub_thmptr(struct crow_packet *pack)
    {
        return crow_packet_dataptr(pack) + sizeof(subheader_pubsub) +
               sizeof(subheader_pubsub_data);
    }

    static inline char *packet_pubsub_datptr(struct crow_packet *pack)
    {
        return crow::packet_pubsub_thmptr(pack) +
               pack->subheader<subheader_pubsub>().thmsz;
    }

    namespace pubsub
    {
        static inline igris::buffer get_theme(crow_packet *pack)
        {
            subheader_pubsub &shps = pack->subheader<subheader_pubsub>();
            return igris::buffer(crow::packet_pubsub_thmptr(pack), shps.thmsz);
        }

        static inline igris::buffer get_data(crow_packet *pack)
        {
            assert(pack->header.f.type == CROW_PUBSUB_PROTOCOL);
            subheader_pubsub_data &shps_d =
                pack->subheader<subheader_pubsub_data>();

            return igris::buffer(crow::packet_pubsub_datptr(pack),
                                 shps_d.datsz);
        }
    }

    class pubsub_packet_ptr : public packet_ptr
    {
      public:
        pubsub_packet_ptr(crow_packet *pack) : packet_ptr(pack) {}

        pubsub_packet_ptr(const crow::packet_ptr &oth) : packet_ptr(oth.get())
        {
        }

        igris::buffer theme() { return pubsub::get_theme(pack); }
        igris::buffer data() { return pubsub::get_data(pack); }
        igris::buffer message() { return pubsub::get_data(pack); }
    };
} // namespace crow

#endif
