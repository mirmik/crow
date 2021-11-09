#ifndef CROW_SUBSCRIBER_NODE_H
#define CROW_SUBSCRIBER_NODE_H

#include <crow/nodes/publisher_node.h>
#include <crow/proto/node.h>
#include <igris/event/delegate.h>

namespace crow
{
    class abstract_subscriber_node : public crow::publisher_node
    {
        int rqos = 2;
        int rackquant = 50;

      public:
        abstract_subscriber_node() = default;
        void
        set_brocker_address(crow::hostaddr_view crowker_addr,
                            int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO);

        void subscribe();
        void subscribe(crow::hostaddr_view crowker_addr, int crowker_node,
                       igris::buffer theme, uint8_t qos, uint16_t ackquant,
                       uint8_t rqos, uint16_t rackquant);

        void subscribe(crow::hostaddr_view crowker_addr, igris::buffer theme);
    };

    class subscriber_node : public crow::abstract_subscriber_node
    {
        igris::delegate<void, igris::buffer> incoming_handler;

      public:
        subscriber_node() = default;
        subscriber_node(igris::delegate<void, igris::buffer> incoming);

        void set_handle(igris::delegate<void, igris::buffer> incoming)
        {
            incoming_handler = incoming;
        }

      private:
        void incoming_packet(crow_packet *) override;
    };
}

#endif