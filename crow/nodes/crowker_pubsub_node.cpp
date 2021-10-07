#include <crow/nodes/crowker_pubsub_node.h>
#include <crow/nodes/pubsub_defs.h>

#include <nos/print.h>

void crow::crowker_pubsub_node::incoming_packet(crow_packet* pack)
{
	auto& s = pack->subheader<pubsub_subheader>();

	switch (s.type)
	{
		case PubSubTypes::Publish:
		{
			auto& sh = pack->subheader<publish_subheader>();
			api->publish_to_theme(sh.theme(), sh.message());
		};
		break;

		case PubSubTypes::Subscribe:
		{
			auto& sh = pack->subheader<subscribe_subheader>();
			api->subscribe_on_theme(pack->addr(), sh.sid, sh.theme(),
			                        sh.rqos, sh.rackquant);
		};
		break;

		default:
			break;
	}

	crow::release(pack);
}
