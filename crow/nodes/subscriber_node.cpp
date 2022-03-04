#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/subscriber_node.h>

void crow::subscriber_node::incoming_packet(crow::packet *pack)
{
    auto &s = pack->subheader<pubsub_subheader>();

    switch (s.type)
    {
    case PubSubTypes::Consume:
    {
        auto &sh = pack->subheader<consume_subheader>();
        incoming_handler(sh.message());
    };
    break;

    default:
        break;
    }

    crow::release(pack);
}

crow::subscriber_node::subscriber_node(
    igris::delegate<void, igris::buffer> incoming)
    : incoming_handler(incoming)
{
}

void crow::abstract_subscriber_node::subscribe(crow::hostaddr_view crowker_addr,
                                               int crowker_node,
                                               igris::buffer theme, uint8_t qos,
                                               uint16_t ackquant, uint8_t rqos,
                                               uint16_t rackquant)
{
    crow::subscribe_subheader sh;

    sh.type = PubSubTypes::Subscribe;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();

    const igris::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                  sizeof(sh) - sizeof(node_subheader)},
                                 theme};

    send_v(crowker_node, crowker_addr, iov, std::size(iov), qos, ackquant);
}

void crow::abstract_subscriber_node::subscribe()
{
    subscribe(crowker_addr, crowker_node, theme, qos, ackquant, rqos, rackquant);
}

void crow::abstract_subscriber_node::subscribe(crow::hostaddr_view crowker_addr,
                                               igris::buffer theme)
{
    subscribe(crowker_addr, crowker_node, theme, qos, ackquant, rqos, rackquant);
}

void crow::abstract_subscriber_node::set_brocker_address(
	crow::hostaddr_view crowker_addr,
    int crowker_node) 
{
	this->crowker_addr = crowker_addr;
	this->crowker_node = crowker_node;
}

void crow::abstract_subscriber_node::subscribe_v2(crow::hostaddr_view crowker_addr,
                       igris::buffer theme, uint8_t request_latest) 
{
    crow::subscribe_subheader_v2 sh;

    sh.type = PubSubTypes::Subscribe_v2;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();
    sh.request_latest = request_latest;

    const igris::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                  sizeof(sh) - sizeof(node_subheader)},
                                 theme};

    send_v(crowker_node, crowker_addr, iov, std::size(iov), qos, ackquant);    
}