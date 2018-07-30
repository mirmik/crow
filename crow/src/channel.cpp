#include <crow/channel.h>
#include <gxx/datastruct/iovec.h>

/*gxx::dlist<crow::channel, &crow::channel::lnk> crow::channels;
*/
void crow::link_channel(crow::channel* ch, uint16_t id) {
	ch->id = id;
	crow::nodes.move_back(*ch);
}
/*
crow::channel* crow::get_channel(uint16_t id) {
	for (auto& ch : crow::channels) {;
		if (ch.id == id) return &ch;
	}
	return nullptr;
}*/
/*
void unknown_port(crow::packet* pack) {
	crow::subheader* rsh = crow::get_subheader(pack);
	crow::subheader sh;

	sh.sid = 0;
	sh.rid = rsh->sid;
	sh.frame_id = 0;
	sh.ftype = crow::Frame::REFUSE;

	crow::send(pack->addrptr(), pack->addrsize(), (const char*)&sh, sizeof(crow::subheader), G1_G2TYPE, crow::QoS(2), pack->header.ackquant);
}
*/
/*void crow::incoming(crow::packet* pack) {
	gxx::println("crow::incomming");
	crow::println(pack);

	crow::subheader* sh = crow::get_subheader(pack);
	crow::channel* ch = get_channel(sh->rid);

	gxx::fprintln("crow subheader: sid={}, rid={}", (uint16_t)sh->sid, (uint16_t)sh->rid);
	dprhex(ch);

	if (ch == nullptr) {
		gxx::println("warn: packet to unrecognized port");
		unknown_port(pack);
		crow::release(pack);
		return;
	}

	switch(sh->ftype) {
		case crow::Frame::HANDSHAKE:
			gxx::println("HANDSHAKE");
			if (ch->state == crow::State::INIT) {
				crow::subheader_handshake* shh = crow::get_subheader_handshake(pack);
				ch->rid = sh->sid;
				ch->qos = shh->qos;
				ch->ackquant = shh->ackquant;
				ch->raddr_ptr = malloc(pack->header.alen);
				memcpy(ch->raddr_ptr, pack->addrptr(), pack->header.alen);
				ch->raddr_len = pack->header.alen;
				ch->state = crow::State::CONNECTED;

			}
			else {
				unknown_port(pack);
			}
			break;
		case crow::Frame::DATA:
			gxx::println("DATA");
			ch->incoming_packet(pack);
			gxx::println("OUT_DATA");
			return;
		case crow::Frame::REFUSE:
			gxx::println("REFUSE");
			ch->state = crow::State::DISCONNECTED;
			break;
		default: break;
	}
	crow::release(pack);
}*/
/*
	gxx::fprintln("crow: incomming for socket {} from remote socket {}", rsock->port, sh->sendport);

	switch( rsock->state ) {
		case crow::crow_socket_state::WAIT_HANDSHAKE: {
			gxx::println("trace: WAIT_HANDSHAKE");
			if (sh->type == crow::crow_frame_type::HANDSHAKE) {
				gxx::println("handshake event");
	
				rsock->raddr_ptr = (uint8_t*)strndup((const char*)pack->addrptr(), pack->addrsize());
				rsock->raddr_len = pack->addrsize();
				rsock->rport = sh->sendport;

				rsock->state = crow::crow_socket_state::BRIDGE;
				if (rsock->handler) rsock->handler(rsock->privdata, crow::event_type::HANDSHAKE);
			} else {
				gxx::println("warn: received DATA frame in WAIT_HANDSHAKE state");
			}
			crow::release(pack);
			return;
		}

		case crow::crow_socket_state::BRIDGE: {
			gxx::println("trace: BRIDGE");
			if (pack->addrsize() == rsock->raddr_len && strncmp((const char*)pack->addrptr(), (const char*)rsock->raddr_ptr, rsock->raddr_len) == 0) {
				//gxx::println("datapackage");
				//rsock->messages.move_back(pack);

				//Добавить в список сообщений в соответствии с порядковым номером.
				add_to_messages_list(rsock, pack, sh);

				if (rsock->handler) rsock->handler(rsock->privdata, crow::event_type::NEWDATA);
				return;
			}
			else {
				gxx::println("warn: message to BRIDGE socket with wrong address");
			}
			crow::release(pack);
			return;
		}

		default:
			gxx::println("warn: unrecognized socket_state state");
	}
*/
//}

void crow::channel::incoming_packet(crow::packet* pack) {
	crow::subheader* sh0 = crow::get_subheader(pack);
	crow::subheader_channel* sh2 = crow::get_subheader_channel(pack);
	
	/*gxx::fprintln("crow subheader: sid={}, rid={}", (uint16_t)sh->sid, (uint16_t)sh->rid);
	dprhex(ch);

	if (ch == nullptr) {
		gxx::println("warn: packet to unrecognized port");
		unknown_port(pack);
		crow::release(pack);
		return;
	}*/

	gxx::println("HERE!!! ", (uint8_t)sh2->ftype);

	switch(sh2->ftype) {
		case crow::Frame::HANDSHAKE:
			gxx::println("HANDSHAKE");
			if (state == crow::State::INIT) {
				crow::subheader_handshake* shh = crow::get_subheader_handshake(pack);
				rid = sh0->sid;
				qos = shh->qos;
				ackquant = shh->ackquant;
				raddr_ptr = malloc(pack->header.alen);
				memcpy(raddr_ptr, pack->addrptr(), pack->header.alen);
				raddr_len = pack->header.alen;
				state = crow::State::CONNECTED;

			}
			else {
				gxx::panic("no INIT state");
				//unknown_port(pack);
			}
			break;
		case crow::Frame::DATA:
			gxx::println("DATA");
			//incoming_packet(pack);
			gxx::println("OUT_DATA");
			return;
		case crow::Frame::REFUSE:
			gxx::println("REFUSE");
			state = crow::State::DISCONNECTED;
			break;
		default: break;
	}
	crow::release(pack);
}

void crow::acceptor::incoming_packet(crow::packet* pack) {
	crow::subheader* sh0 = crow::get_subheader(pack);
	crow::subheader_channel* sh2 = crow::get_subheader_channel(pack);
	crow::subheader_handshake* shh = crow::get_subheader_handshake(pack);

	auto ch = init_channel();

	crow::handshake(ch, sh0->sid, pack->addrptr(), pack->addrsize(), shh->qos, shh->ackquant);
	crow::release(pack);
}

void crow::handshake(crow::channel* ch, uint16_t rid, const void* raddr_ptr, size_t raddr_len, crow::QoS qos, uint16_t ackquant) {
	crow::subheader sh0;
	crow::subheader_channel sh2;
	crow::subheader_handshake shh;

	sh0.sid = ch->id;
	sh0.rid = ch->rid = rid;

	sh2.frame_id = 0;
	sh2.ftype = crow::Frame::HANDSHAKE;

	ch->raddr_ptr = malloc(raddr_len);
	memcpy(ch->raddr_ptr, raddr_ptr, raddr_len);
	ch->raddr_len = raddr_len;

	ch->qos = shh.qos = qos;
	ch->ackquant = shh.ackquant = ackquant;

	gxx::iovec vec[] = {
		{&sh0, sizeof(sh0)},
		{&sh2, sizeof(sh2)},
		{&shh, sizeof(shh)},
	};

	ch->state = crow::State::CONNECTED;
	crow::send(raddr_ptr, raddr_len, vec, sizeof(vec) / sizeof(gxx::iovec), G1_G0TYPE, crow::QoS(2), ackquant);
}

void crow::__channel_send(crow::channel* ch, const char* data, size_t size) {
	crow::subheader sh0;
	crow::subheader_channel sh2;
	sh0.sid = ch->id;
	sh0.rid = ch->rid;
	sh2.frame_id = ch->fid++;
	sh2.ftype = crow::Frame::DATA;	

	gxx::iovec vec[] = {
		{&sh0, sizeof(sh0)},
		{&sh2, sizeof(sh2)},
		{data, size},
	};
	crow::send(ch->raddr_ptr, ch->raddr_len, vec, sizeof(vec) / sizeof(gxx::iovec), G1_G0TYPE, ch->qos, ch->ackquant);
}

uint16_t crow::dynport() {
	return 512;
}