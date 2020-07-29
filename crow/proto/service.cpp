#include <crow/proto/service.h>

crow::packet_ptr crow::service_packet_ptr::answer(igris::buffer data)
{
	auto subheader = node::subheader(this->pack);
	return srv->send(subheader->sid, pack->addr(), data, pack->qos(), pack->ackquant());
}