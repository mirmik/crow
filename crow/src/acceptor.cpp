#include <crow/channel.h>
#include <crow/tower.h>

void crow::acceptor::incoming_packet(crow::packet *pack)
{
	crow::node_subheader *sh0 = (node_subheader *) pack->dataptr();
	crow::subheader_handshake *shh = crow::get_subheader_handshake(pack);

	auto ch = init_channel();
	link_channel(ch, crow::dynport());

	DPRINT(sh0->sid);

	ch->handshake(pack->addrptr(), pack->addrsize(), sh0->sid, 
		shh->qos, shh->ackquant);
	crow::release(pack);
}

void crow::acceptor::undelivered_packet(crow::packet *pack) 
{
	crow::release(pack);
}
