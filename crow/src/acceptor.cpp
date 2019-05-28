#include <crow/channel.h>
#include <crow/tower.h>

#include <nos/trace.h>

void crow::acceptor::incoming_packet(crow::packet *pack)
{
	TRACE();
	crow::node_subheader *sh0 = (node_subheader *) pack->dataptr();
	crow::subheader_handshake *shh = crow::get_subheader_handshake(pack);

	auto ch = init_channel();
	if (ch->id == 0)
		link_channel(ch, dynport());

	ch->handshake(pack->addrptr(), pack->addrsize(), sh0->sid, 
		shh->qos, shh->ackquant);

	crow::release(pack);
}

void crow::acceptor::undelivered_packet(crow::packet *pack) 
{
	TRACE();
	crow::release(pack);
}
