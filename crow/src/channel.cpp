#include <crow/channel.h>
#include <crow/tower.h>
#include <sys/uio.h>

#include <igris/util/bug.h>
#include <nos/print.h>

void crow::link_channel(crow::channel *ch, uint16_t id)
{
	link_node(ch, id);
}

void crow::channel::incoming_packet(crow::packet *pack)
{
	crow::node_subheader *sh0 = (node_subheader *) pack->dataptr();
	crow::subheader_channel *sh2 = crow::get_subheader_channel(pack);

	switch (sh2->ftype)
	{
		case crow::Frame::HANDSHAKE:
			nos::println("HANDSHAKE");

			if (state == crow::State::INIT)
			{
				crow::subheader_handshake *shh =
				    crow::get_subheader_handshake(pack);
				rid = sh0->sid;
				qos = shh->qos;
				ackquant = shh->ackquant;
				raddr_ptr = malloc(pack->header.alen);
				memcpy(raddr_ptr, pack->addrptr(), pack->header.alen);
				raddr_len = pack->header.alen;
				state = crow::State::CONNECTED;
			}
			else
			{
				BUG_ON("no INIT state");
				// unknown_port(pack);
			}

			break;

		case crow::Frame::DATA:
			nos::println("DATA");
			incoming_data_packet(pack);
			return;

		case crow::Frame::REFUSE:
			nos::println("REFUSE");
			state = crow::State::DISCONNECTED;
			break;

		default:
			break;
	}

	crow::release(pack);
}

void crow::channel::incoming_data_packet(crow::packet * pack)
{
	crow::release(pack);
}

void crow::channel::undelivered_packet(crow::packet * pack)
{
	crow::release(pack);
}

void crow::channel::handshake(const uint8_t *raddr_ptr, uint16_t raddr_len, uint16_t rid, 
	uint8_t qos, uint16_t ackquant)
{
	crow::node_subheader sh0;
	crow::subheader_channel sh2;
	crow::subheader_handshake shh;

	sh0.sid = id;
	sh0.rid = this->rid = rid;

	sh2.frame_id = 0;
	sh2.ftype = crow::Frame::HANDSHAKE;

	this->raddr_ptr = malloc(raddr_len);
	memcpy(this->raddr_ptr, raddr_ptr, raddr_len);
	this->raddr_len = raddr_len;

	this->qos = shh.qos = qos;
	this->ackquant = shh.ackquant = ackquant;

	iovec vec[] =
	{
		{&sh0, sizeof(sh0)},
		{&sh2, sizeof(sh2)},
		{&shh, sizeof(shh)},
	};

	state = crow::State::CONNECTED;
	crow::send_v(raddr_ptr, raddr_len, vec, sizeof(vec) / sizeof(iovec),
	             CROW_NODE_PROTOCOL, 2, ackquant);
}

void crow::__channel_send(crow::channel *ch, const char *data, size_t size)
{
	crow::node_subheader sh0;
	crow::subheader_channel sh2;
	sh0.sid = ch->id;
	sh0.rid = ch->rid;
	sh2.frame_id = ch->fid++;
	sh2.ftype = crow::Frame::DATA;

	iovec vec[] =
	{
		{&sh0, sizeof(sh0)},
		{&sh2, sizeof(sh2)},
		{(void *)data, size},
	};
	crow::send_v(ch->raddr_ptr, ch->raddr_len, vec, sizeof(vec) / sizeof(iovec),
	             CROW_NODE_PROTOCOL, ch->qos, ch->ackquant);
}

uint16_t crow::dynport() { return 512; }

void crow::channel::send(const char *data, size_t size)
{
	crow::__channel_send(this, data, size);
}


igris::buffer crow::channel::getdata(crow::packet *pack)
{
	return igris::buffer(pack->dataptr() + sizeof(crow::node_subheader) +
	                     sizeof(crow::subheader_channel),
	                     pack->datasize() - sizeof(crow::node_subheader) -
	                     sizeof(crow::subheader_channel));
}