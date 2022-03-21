#ifndef CROW_NODE_PUBLISHER_H
#define CROW_NODE_PUBLISHER_H

#include <crow/hostaddr.h>
#include <crow/proto/node.h>

namespace crow
{
    class publisher_node : public crow::node
    {
    protected:
        crow::hostaddr crowker_addr={};
        int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO;
        int qos = 0;
        int ackquant = 0;

        std::string theme={};

    public:
        publisher_node() = default;

        publisher_node(crow::hostaddr_view crowker_addr, int crowker_node,
                       igris::buffer theme);
        publisher_node(crow::hostaddr_view crowker_addr, igris::buffer theme);

        void init(crow::hostaddr_view crowker_addr, igris::buffer theme) 
        {
            set_theme(theme);
            set_address(crowker_addr);
        }

        void publish(igris::buffer data);
        void publish_v(igris::buffer *data, int len);
        void publish(crow::hostaddr_view addr, int crowker_node,
                     igris::buffer theme, igris::buffer data, int qos,
                     int ackquant);

        void set_theme(igris::buffer theme);
        void set_address(crow::hostaddr_view addr);

        void set_qos(int _qos, int _ackquant)
        {
            qos = _qos;
            ackquant = _ackquant;
        }

    private:
        void incoming_packet(crow::packet *pack) override
        {
            crow::release(pack);
        }
    };

    using publisher = publisher_node;
}

#endif
