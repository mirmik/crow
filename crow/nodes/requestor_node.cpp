#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/requestor_node.h>
#include <nos/print.h>

void crow::requestor_node::incoming_packet(crow::packet *pack)
{
    if (incoming.armed())
    {
        auto &s = pack->subheader<pubsub_subheader>();

        switch (s.type)
        {
            case PubSubTypes::Consume:
            {
                auto &sh = pack->subheader<consume_subheader>();
                incoming(sh.message());
            };
            break;

            default:
                break;
        }

        crow::release(pack);
    }

    else
    {
        dlist_move(&pack->ulnk, &incoming_list);
        notify_one(0);
    }
}

void crow::requestor_node::async_request(crow::hostaddr_view crowker_addr,
        nodeid_t crowker_node, igris::buffer theme,
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
    async_request(data);
}

void crow::requestor_node::async_request(igris::buffer data)
{
    crow::request_subheader sh;

    sh.type = PubSubTypes::Request;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();
    sh.datsize = data.size();
    sh.repthmsize = reply_theme.size();

    const igris::buffer iov[] = {{
            (char *)&sh + sizeof(node_subheader),
            sizeof(sh) - sizeof(node_subheader)
        },
        theme,
        reply_theme,
        data
    };

    send_v(crowker_node, crowker_addr, iov, std::size(iov), qos, ackquant);
}

crow::requestor_node::requestor_node(
    crow::hostaddr_view crowker_addr,
    igris::buffer theme,
    igris::buffer reptheme)
    : publisher_node(crowker_addr, theme), reply_theme(reptheme)
{}

crow::requestor_node::requestor_node(
    crow::hostaddr_view crowker_addr,
    nodeid_t crowker_node,
    igris::buffer theme,
    igris::buffer reptheme)
    : publisher_node(crowker_addr, crowker_node, theme), reply_theme(reptheme)
{}

void crow::requestor_node::set_reply_theme(igris::buffer reply_theme)
{
    this->reply_theme = reply_theme;
}