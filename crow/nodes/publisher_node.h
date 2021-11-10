#ifndef CROW_NODE_PUBLISHER_H
#define CROW_NODE_PUBLISHER_H

#include <crow/proto/node.h>

namespace crow
{
    class publisher_node : public crow::node
    {
    protected:
        igris::buffer theme;
        crow::hostaddr_view crowker_addr;
        int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO;

        int qos = 0;
        int ackquant = 0;

    public:
        publisher_node(igris::buffer theme, crow::hostaddr_view crowker_addr,
                       int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO);

        publisher_node() = default;

        publisher_node(const publisher_node &) = default;

        void publish(igris::buffer data);
        void publish(crow::hostaddr_view addr, int crowker_node,
                     igris::buffer theme, igris::buffer data, int qos,
                     int ackquant);

        void set_theme(igris::buffer theme);

    private:
        void incoming_packet(crow::packet *pack) override
        {
            crow::release(pack);
        }
    };
}

#endif