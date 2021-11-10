#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/request_node.h>
#include <nos/print.h>

void crow::request_node::incoming_packet(crow::packet *pack)
{
    dlist_move(&pack->ulnk, &incoming_list);
    notify_one(0);
}

void crow::request_node::request(crow::hostaddr_view crowker_addr,
                                 int crowker_node, igris::buffer theme,
                                 igris::buffer reptheme, igris::buffer data,
                                 uint8_t qos, uint16_t ackquant, uint8_t rqos,
                                 uint16_t rackquant)
{
    this->crowker_node = crowker_node;
    this->crowker_addr = crowker_addr;
    this->qos = qos;
    this->rqos = rqos;
    this->ackquant = ackquant;
    this->rackquant = rackquant;
    this->theme = theme;
    this->reply_theme = reptheme;
    request(data);
}

void crow::request_node::request(igris::buffer data)
{
    crow::request_subheader sh;

    sh.type = PubSubTypes::Request;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.keepalive = keepalive;
    sh.thmsize = theme.size();
    sh.datsize = data.size();
    sh.repthmsize = reply_theme.size();

    const igris::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                  sizeof(sh) - sizeof(node_subheader)},
                                 theme,
                                 reply_theme,
                                 data};

    send_v(crowker_node, crowker_addr, iov, std::size(iov), qos, ackquant);
}

crow::packet_ptr crow::request_node::sync_request(igris::buffer data)
{
    request(data);
    int sts = waitevent();
    (void)sts;

    auto *ptr = dlist_first_entry(&incoming_list, crow::packet, ulnk);
    return crow::packet_ptr(ptr);
}