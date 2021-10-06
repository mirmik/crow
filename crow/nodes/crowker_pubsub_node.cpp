#include <crow/nodes/crowker_pubsub_node.h>
#include <crow/nodes/pubsub_defs.h>

#include <nos/print.h>

void crow::crowker_pubsub_node::incoming_packet(crow_packet* pack)
{
	nos::println("crowker_pubsub_node::incoming_packet");
	auto& s = pack->subheader<pubsub_subheader>();

	switch (s.type)
	{
		case PubSubTypes::Publish:
		{
			nos::println("crowker_pubsub_node::incoming_packet:Publish");
			auto& sh = pack->subheader<publish_subheader>();
			api->publish_to_theme(sh.theme(), sh.message());
		};
		break;

		case PubSubTypes::Subscribe:
		{
			nos::println("crowker_pubsub_node::incoming_packet:Subscribe");
			auto& sh = pack->subheader<publish_subheader>();
			api->subscribe_on_theme(pack->addr(), sh.sid, sh.theme());
		};
		break;

		default:
			break;
	}

	crow::release(pack);
}
