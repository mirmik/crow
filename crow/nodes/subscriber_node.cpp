#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/subscriber_node.h>

void crow::subscriber_node::incoming_packet(crow_packet* pack)
{
	auto& s = pack->subheader<pubsub_subheader>();

	switch (s.type)
	{
		case PubSubTypes::Consume:
		{
			auto& sh = pack->subheader<consume_subheader>();
			incoming_handler(sh.message());
		};
		break;

		default:
			break;
	}

	crow::release(pack);
}

crow::subscriber_node::subscriber_node(igris::delegate<void, igris::buffer> incoming)
	: incoming_handler(incoming)
{}