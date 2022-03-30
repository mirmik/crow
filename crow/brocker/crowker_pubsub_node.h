#ifndef CROW_NODES_CROWKER_PUBSUB_NODE_H
#define CROW_NODES_CROWKER_PUBSUB_NODE_H

#include <crow/proto/node.h>
#include <memory>

namespace crow
{
    class crowker_pubsub_node_api;

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