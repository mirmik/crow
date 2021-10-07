#ifndef CROW_SUBSCRIBER_NODE_H
#define CROW_SUBSCRIBER_NODE_H

#include <crow/proto/node.h>
#include <igris/event/delegate.h>

namespace crow
{
    class subscriber_node : public crow::node
    {
        igris::delegate<void, igris::buffer> incoming_handler;
        int qos = 2;
        int ackquant = 50;
        int rqos = 2;
        int rackquant = 50;
        int keepalive = 30;

        igris::buffer theme;
        crow::hostaddr_view crowker_addr;
        int crowker_node;

      public:
        subscriber_node() = default;
        subscriber_node(igris::delegate<void, igris::buffer> incoming);

        void set_brocker_address(crow::hostaddr_view crowker_addr,
                                 int crowker_node);
        void set_theme(igris::buffer theme);

        void subscribe();
        void subscribe(crow::hostaddr_view crowker_addr, int crowker_node,
                       igris::buffer theme, uint8_t qos, uint16_t ackquant,
                       uint8_t rqos, uint16_t rackquant);

      private:
        void incoming_packet(crow_packet *) override;
    };
}

#endif