#ifndef CROW_NODE_PUBLISHER_H
#define CROW_NODE_PUBLISHER_H

#include <crow/proto/node.h>

namespace crow
{
    class publisher_node : public crow::node
    {
        igris::buffer theme;
        crow::hostaddr_view crowker_addr;
        int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO;

        int qos = 2;
        int ackquant = 50;

      public:
        void publish(igris::buffer data);
        void publish(crow::hostaddr_view addr, int crowker_node,
                     igris::buffer theme, igris::buffer data, int qos,
                     int ackquant);

      private:
        void incoming_packet(crow_packet *pack) override
        {
            crow::release(pack);
        }
    };
}

#endif