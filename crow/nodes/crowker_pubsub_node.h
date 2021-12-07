#ifndef CROW_NODES_CROWKER_PUBSUB_NODE_H
#define CROW_NODES_CROWKER_PUBSUB_NODE_H

#include <crow/proto/node.h>

namespace crow
{
    class crowker_pubsub_node_api
    {
    public:
        virtual void subscribe_on_theme(crow::hostaddr_view, int nid,
                                        igris::buffer theme, uint8_t rqos,
                                        uint16_t rackquant) = 0;

        virtual void publish_to_theme(igris::buffer theme,
                                      igris::buffer data) = 0;

        virtual void undelivered_packet(crow::hostaddr_view, int node) = 0;
        virtual void client_beam(crow::hostaddr_view, int nid,
                                 igris::buffer name) = 0;
    };

    class crowker_pubsub_node : public crow::node
    {
        crowker_pubsub_node_api *api;

    public:
        void set_api(crowker_pubsub_node_api *api) { this->api = api; }

    private:
        void incoming_packet(crow::packet *) override;
        void undelivered_packet(crow::packet *) override;
    };
}

#endif