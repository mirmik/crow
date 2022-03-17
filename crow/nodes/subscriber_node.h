#ifndef CROW_SUBSCRIBER_NODE_H
#define CROW_SUBSCRIBER_NODE_H

#include <crow/nodes/publisher_node.h>
#include <crow/proto/node.h>
#include <igris/event/delegate.h>
#include <nos/print.h>

namespace crow
{
    class abstract_subscriber_node : public crow::publisher_node, public crow::alived_object
    {
    protected:
        int rqos = 0;
        int rackquant = 0;

    public:
        abstract_subscriber_node(crow::hostaddr_view addr, int node,
                       igris::buffer theme) : publisher_node(addr, node, theme) {}
        abstract_subscriber_node(crow::hostaddr_view addr, igris::buffer theme) : publisher_node(addr, theme) {}

        abstract_subscriber_node() = default;
        void
        set_brocker_address(crow::hostaddr_view crowker_addr,
                            int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO);

        void set_rqos(int rqos, int rackquant) { this->rqos = rqos; this->rackquant = rackquant; }

        void subscribe();
        void subscribe(crow::hostaddr_view crowker_addr, int crowker_node,
                       igris::buffer theme, uint8_t qos, uint16_t ackquant,
                       uint8_t rqos, uint16_t rackquant);

        void subscribe_v2(crow::hostaddr_view crowker_addr,
                       igris::buffer theme, uint8_t get_latest);

        void subscribe(crow::hostaddr_view crowker_addr, igris::buffer theme);
        void keepalive_handle() override 
        { 
            subscribe(); 
        }
    };

    class subscriber_node : public crow::abstract_subscriber_node
    {
        igris::delegate<void, igris::buffer> incoming_handler = {};

    public:
        subscriber_node() = default;
        subscriber_node(igris::delegate<void, igris::buffer> incoming);

        void set_handle(igris::delegate<void, igris::buffer> incoming)
        {
            incoming_handler = incoming;
        }

    private:
        void incoming_packet(crow::packet *) override;
    };

    using subscriber = subscriber_node;
}

#endif