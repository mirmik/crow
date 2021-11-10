#ifndef CROW_REQUEST_NODE_H
#define CROW_REQUEST_NODE_H

#include <crow/proto/node.h>

namespace crow
{
    class request_node : public crow::node
    {
        dlist_head incoming_list = DLIST_HEAD_INIT(incoming_list);

        igris::buffer theme;
        igris::buffer reply_theme;
        crow::hostaddr_view crowker_addr;
        nodeid_t crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO;

        int qos = 0;
        int ackquant = 0;
        int rqos = 0;
        int rackquant = 0;

      public:
        void request(crow::hostaddr_view crowker_addr, nodeid_t crowker_node,
                     igris::buffer theme, igris::buffer reptheme,
                     igris::buffer data, uint8_t qos, uint16_t ackquant,
                     uint8_t rqos, uint16_t rackquant);

        void request(igris::buffer data);
        crow::packet_ptr sync_request(igris::buffer data);

      private:
        void incoming_packet(crow::packet *pack) override;
    };
}

#endif