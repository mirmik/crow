#ifndef CROW_requestor_node_H
#define CROW_requestor_node_H

#include <crow/nodes/publisher_node.h>
#include <crow/proto/node.h>

namespace crow
{
    class requestor_node : public crow::publisher_node
    {
        dlist_head incoming_list = DLIST_HEAD_INIT(incoming_list);
        igris::buffer reply_theme;

        int qos = 0;
        int ackquant = 0;
        int rqos = 0;
        int rackquant = 0;

      public:
        requestor_node() = default;
        requestor_node(crow::hostaddr_view crowker_addr, igris::buffer theme,
                       igris::buffer reptheme);
        requestor_node(crow::hostaddr_view crowker_addr, nodeid_t crowker_node,
                       igris::buffer theme, igris::buffer reptheme);

        void async_request(crow::hostaddr_view crowker_addr,
                           nodeid_t crowker_node, igris::buffer theme,
                           igris::buffer reptheme, igris::buffer data,
                           uint8_t qos, uint16_t ackquant, uint8_t rqos,
                           uint16_t rackquant);

        void async_request(igris::buffer data);

        template <class... Args> crow::packet_ptr request(Args &&...data)
        {
            async_request(std::forward<Args>(data)...);
            int sts = waitevent();
            (void)sts;

            auto *ptr = dlist_first_entry(&incoming_list, crow::packet, ulnk);
            return crow::packet_ptr(ptr);
        }

        void set_reply_theme(igris::buffer reply_theme);

      private:
        void incoming_packet(crow::packet *pack) override;
    };
}

#endif