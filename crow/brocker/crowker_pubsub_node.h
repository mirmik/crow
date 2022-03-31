#ifndef CROW_NODES_CROWKER_PUBSUB_NODE_H
#define CROW_NODES_CROWKER_PUBSUB_NODE_H

#include <crow/proto/node.h>
#include <crow/brocker/crowker_api.h>
#include <memory>

namespace crow
{
    class crowker_pubsub_node : public crow::node, public crowker_api
    {
        class node_client : public crowker_implementation::client
        {
        public:
            struct options_struct
            {
                uint8_t qos=0;
                uint16_t ackquant=0;
            };

            crowker_api *api=nullptr;
            crowker_pubsub_node *crowker_node=nullptr;

            crow::hostaddr addr={};
            int node=-1;

            std::map<std::string, options_struct> options={};

        public:
            void publish(const std::string &theme, const std::string &data,
                 crowker_implementation::options *opts) override;

            ~node_client();
        };

        std::map<std::pair<crow::hostaddr, int>, node_client> clients={};

    private:
        std::vector<crowker_implementation::client *> get_clients() override;

        void subscribe_on_theme(crow::hostaddr_view, int nid,
                                igris::buffer theme, uint8_t rqos,
                                uint16_t rackquant) override;
        void unsubscribe_from_theme(crow::hostaddr_view view, int nid, igris::buffer theme);
        void subscribe_on_theme_v2(crow::hostaddr_view, int nid,
                                igris::buffer theme, uint8_t rqos,
                                uint16_t rackquant, bool subscribe_on_updates, 
                                uint32_t request_latest) override;
        void client_beam(crow::hostaddr_view, int nid,
                         igris::buffer name) override;
        void undelivered_packet_handle(crow::hostaddr_view, int node) override;

        void incoming_packet(crow::packet *) override;
        void undelivered_packet(crow::packet *) override;
    };
}

#endif