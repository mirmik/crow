#include <crow/brocker/crowker_api.h>
#include <crow/nodes/pubsub_defs.h>
#include <igris/util/bug.h>

void crow::crowker_api::subscribe_on_theme(crow::hostaddr_view view, int nid,
        igris::buffer theme, uint8_t rqos, uint16_t rackquant)
{
	crowker_api_client::options_struct opt = { rqos, rackquant };

	auto key = std::make_pair(view, nid);

	if (clients.count(key))
	{
		return;
	}

	auto& client = clients[key];

	client.api = this;
	client.addr = view;
	client.node = nid;
	client.crowker_node = crowker_node;

	client.options[theme.to_string()] = opt;
	crowker->subscribe(theme.to_string(), &client);
}

void crow::crowker_api::publish_to_theme(igris::buffer theme,
        igris::buffer data)
{
	crowker->publish(theme.to_string(), data.to_string());
}


crow::crowker_api_client::~crowker_api_client()
{

}

void crow::crowker_api_client::publish(
    const std::string &theme,
    const std::string &data,
    crowker_implementation::options*)
{
	const auto & opts = options[theme];
	crow::consume_subheader sh;

	sh.type = PubSubTypes::Consume;
	sh.thmsize = theme.size();
	sh.datsize = data.size();

	const igris::buffer iov[] =
	{
		{(char*)&sh + sizeof(node_subheader), sizeof(sh) - sizeof(node_subheader)},
		theme,
		data,
	};

	crowker_node->send_v(
	    node,
	    addr,
	    iov, std::size(iov),
	    opts.qos, opts.ackquant);

}