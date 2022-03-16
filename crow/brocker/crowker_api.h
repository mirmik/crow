#ifndef CROW_CROWKER_API_H
#define CROW_CROWKER_API_H

#include <crow/brocker/client.h>
#include <crow/brocker/crowker.h>
#include <crow/nodes/crowker_pubsub_node.h>

namespace crow
{
    class crowker_api;

    class crowker_api_client : public client
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

        ~crowker_api_client();
    };

    class crowker_api : public crowker_pubsub_node_api
    {
        std::map<std::pair<crow::hostaddr, int>, crowker_api_client> clients={};

    public:
        crowker_pubsub_node *crowker_node=nullptr;
        crow::crowker *crowker=nullptr;

        std::vector<crowker_implementation::client *> get_clients()
        {
            std::vector<crowker_implementation::client *> ret;
            for (auto &pair : clients)
                ret.push_back(&pair.second);
            return ret;
        }

    public:
        void subscribe_on_theme(crow::hostaddr_view, int nid,
                                igris::buffer theme, uint8_t rqos,
                                uint16_t rackquant) override;

        void client_beam(crow::hostaddr_view, int nid,
                         igris::buffer name) override;

        void publish_to_theme(igris::buffer theme, const std::shared_ptr<std::string>& data) override;
        void undelivered_packet(crow::hostaddr_view, int node) override;
    };
}

#endif