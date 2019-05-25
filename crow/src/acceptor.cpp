#include <crow/channel.h>
#include <crow/tower.h>

void crow::acceptor::incoming_packet(crow::packet *pack)
{
	crow::node_subheader *sh0 = (node_subheader *) pack->dataptr();
	crow::subheader_handshake *shh = crow::get_subheader_handshake(pack);

	auto ch = init_channel();

	crow::handshake(ch, sh0->sid, pack->addrptr(), pack->addrsize(), shh->qos,
					shh->ackquant);
	crow::release(pack);
}

void crow::acceptor::undelivered_packet(crow::packet *pack) 
{
	crow::release(pack);
}
