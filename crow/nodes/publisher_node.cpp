#include <crow/nodes/publisher_node.h>
#include <crow/nodes/pubsub_defs.h>


void crow::publisher_node::publish_v(igris::buffer * data, int len)
{
    crow::publish_subheader sh;

    sh.type = PubSubTypes::Publish;
    sh.thmsize = theme.size();

    const igris::buffer iov[] =
    {
        {
            (char *)&sh + sizeof(node_subheader),
            sizeof(sh) - sizeof(node_subheader)
        },
    };

    send_vv(crowker_node, crowker_addr, iov, std::size(iov), data, len, qos, ackquant);    
}

void crow::publisher_node::publish(igris::buffer data)
{
    crow::publish_subheader sh;

    sh.type = PubSubTypes::Publish;
    sh.thmsize = theme.size();
    sh.datsize = data.size();

    const igris::buffer iov[] =
    {
        {
            (char *)&sh + sizeof(node_subheader),
            sizeof(sh) - sizeof(node_subheader)
        },
        theme,
        data,
    };

    send_v(crowker_node, crowker_addr, iov, std::size(iov), qos, ackquant);
}

void crow::publisher_node::publish(crow::hostaddr_view addr, int crowker_node,
                                   igris::buffer theme, igris::buffer data,
                                   int qos, int ackquant)
{
    this->crowker_addr = addr;
    this->crowker_node = crowker_node;
    this->theme = theme.to_string();
    this->qos = qos;
    this->ackquant = ackquant;

    publish(data);
}

void crow::publisher_node::set_theme(igris::buffer theme)
{
    this->theme = theme.to_string();
}

void crow::publisher_node::set_address(crow::hostaddr_view addr)
{
    this->crowker_addr = addr;
}

crow::publisher_node::publisher_node(
    crow::hostaddr_view crowker_addr, int crowker_node, igris::buffer theme)
    : theme(theme), crowker_addr(crowker_addr), crowker_node(crowker_node)
{}

crow::publisher_node::publisher_node(
    crow::hostaddr_view crowker_addr, igris::buffer theme)
    : theme(theme), crowker_addr(crowker_addr), crowker_node(CROWKER_SERVICE_BROCKER_NODE_NO)
{}