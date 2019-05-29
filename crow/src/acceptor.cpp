#include <crow/acceptor.h>
#include <crow/channel.h>
#include <crow/tower.h>

#include <nos/trace.h>

void crow::acceptor::incoming_packet(crow::packet *pack)
{
	TRACE();
	//crow::node_subheader *sh0 = (node_subheader *) pack->dataptr();
	crow::subheader_channel *shc = crow::get_subheader_channel(pack);

	if (shc->ftype == Frame::HANDSHAKE_REQUEST)
	{
		auto ch = init_channel();
		if (ch->id == 0)
			link_channel(ch, dynport());

		ch->wait_handshake_request();
		ch->incoming_packet(pack);
	}
	else
		crow::release(pack);
}

void crow::acceptor::undelivered_packet(crow::packet *pack)
{
	TRACE();
	crow::release(pack);
}

uint16_t crow::dynport()
{
	return 512;
}