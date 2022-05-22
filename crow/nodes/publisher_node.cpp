#include <crow/nodes/publisher_node.h>
#include <crow/nodes/pubsub_defs.h>


void crow::abstract_publisher_node::publish_v(crow::hostaddr_view addr, int node,
                                   igris::buffer theme, igris::buffer * data, int len,
                                   int qos, int ackquant)
{
    crow::publish_subheader sh;

    sh.type = PubSubTypes::Publish;
    sh.thmsize = theme.size();
    sh.datsize = 0;
    for (int i = 0; i < len; i++)
        sh.datsize += data[i].size();

    const igris::buffer iov[] =
    {
        {
            (char *)&sh + sizeof(node_subheader),
            sizeof(sh) - sizeof(node_subheader),
        },
        theme,
    };

    node::send_vv(node, addr, iov, std::size(iov), data, len, qos, ackquant, _async);
}

void crow::abstract_publisher_node::publish(crow::hostaddr_view addr, int node,
                                   igris::buffer theme, igris::buffer data,
                                   int qos, int ackquant)
{
    abstract_publisher_node::publish_v(addr, node, theme, &data, 1, qos, ackquant);
}


void crow::publisher_node::publish_v(igris::buffer * data, int len) 
{
    abstract_publisher_node::publish_v(crowker_addr, crowker_node, theme, data, len, qos, ackquant);
}

void crow::publisher_node::publish(igris::buffer data)
{
    abstract_publisher_node::publish_v(crowker_addr, crowker_node, theme, &data, 1, qos, ackquant);
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
    :
    crowker_addr(crowker_addr),
    crowker_node(crowker_node),
    theme(theme)
{}

crow::publisher_node::publisher_node(
    crow::hostaddr_view crowker_addr, igris::buffer theme)
    :
    crowker_addr(crowker_addr),
    crowker_node(CROWKER_SERVICE_BROCKER_NODE_NO),
    theme(theme)
{}