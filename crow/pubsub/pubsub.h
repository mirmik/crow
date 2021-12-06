/** @file */

#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <assert.h>

#include <igris/buffer.h>
#include <igris/event/delegate.h>
#include <igris/sync/syslock.h>

#include <crow/proto/protocol.h>
#include <crow/tower.h>

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

    struct subheader_pubsub_data : public subheader_pubsub
    {
        uint16_t datsz;
        igris::buffer theme() { return {(char *)(this + 1), thmsz}; }
        igris::buffer data() { return {(char *)(this + 1) + thmsz, datsz}; }
    } __attribute__((packed));

    struct subheader_pubsub_control : public subheader_pubsub
    {
        uint8_t qos;
        uint16_t ackquant;
        igris::buffer theme() { return {(char *)(this + 1), thmsz}; }
    } __attribute__((packed));

    class pubsub_protocol_cls : public crow::protocol
    {
      public:
        struct dlist_head subscribers = DLIST_HEAD_INIT(subscribers);
        void (*incoming_handler)(crow::packet *);
        void (*undelivered_handler)(crow::packet *);

      public:
        pubsub_protocol_cls() {}

        void incoming(crow::packet *pack) override;
        void undelivered(crow::packet *pack) override;

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

    class pubsub_packet_ptr : public packet_ptr
    {
      public:
        pubsub_packet_ptr(crow::packet *pack) : packet_ptr(pack) {}

        pubsub_packet_ptr(const crow::packet_ptr &oth) : packet_ptr(oth.get())
        {
        }

        igris::buffer theme()
        {
            auto &subheader = pack->subheader<subheader_pubsub_data>();
            return subheader.theme();
        }

        igris::buffer message()
        {
            auto &subheader = pack->subheader<subheader_pubsub_data>();
            return subheader.data();
        }
    };
} // namespace crow

#endif
