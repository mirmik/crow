/**
@file tower.cpp
*/

#include <crow/tower.h>
#include <crow/host.h>
#include <gxx/util/hexascii.h>
#include <gxx/algorithm.h>
#include <gxx/util/string.h>
#include <gxx/syslock.h>
#include <gxx/util/hexer.h>
#include <gxx/print/stdprint.h>

gxx::dlist<crow::gateway, &crow::gateway::lnk> crow::gateways;
gxx::dlist<crow::packet, &crow::packet::lnk> crow::travelled;
gxx::dlist<crow::packet, &crow::packet::lnk> crow::incoming;
gxx::dlist<crow::packet, &crow::packet::lnk> crow::outters;
void(*crow::user_type_handler)(crow::packet* pack) = nullptr;
void(*crow::user_incoming_handler)(crow::packet* pack) = nullptr;
void(*crow::undelivered_handler)(crow::packet* pack) = nullptr;
void(*crow::traveling_handler)(crow::packet* pack) = nullptr;
void(*crow::transit_handler)(crow::packet* pack) = nullptr;

static void __traveling_handler(crow::packet* pack) {
	gxx::print("travel: "); crow::println(pack);
}

void crow::enable_diagnostic() {
	crow::traveling_handler = __traveling_handler;
}

crow::gateway* crow::find_target_gateway(const crow::packet* pack) {
	uint8_t gidx = *pack->stageptr();
	for (auto& g : gateways) {
		if (g.id == gidx) return &g;
	}
	return nullptr;
}

void crow::release(crow::packet* pack) {
	gxx::system_lock();
	if (pack->released_by_tower) crow::utilize(pack);
	else pack->released_by_world = true;
	gxx::system_unlock();
}

void crow::tower_release(crow::packet* pack) {
	gxx::syslock().lock();
	dlist_del(&pack->lnk);
	if (pack->released_by_world) crow::utilize(pack);
	else pack->released_by_tower = true;
	gxx::syslock().unlock();
}

void utilize_from_outers(crow::packet* pack) {
	for (auto& el : crow::outters) {
		if (
			el.header.seqid == pack->header.seqid && 
			pack->header.alen == el.header.alen && 
			!memcmp(el.addrptr(), pack->addrptr(), pack->header.alen)
		) {
			crow::utilize(&el);
			return;
		}
	}
}

void qos_release_from_incoming(crow::packet* pack) {
	for (auto& el : crow::incoming) {
		if (el.header.seqid == pack->header.seqid && 
			pack->header.alen == el.header.alen && 
			!memcmp(el.addrptr(), pack->addrptr(), pack->header.alen)
		) {
			crow::tower_release(&el);
			return;
		}
	}
}

void add_to_incoming_list(crow::packet* pack) {
	pack->last_request_time = crow::millis();
	crow::incoming.move_back(*pack);
}

void add_to_outters_list(crow::packet* pack) {
	pack->last_request_time = crow::millis();
	crow::outters.move_back(*pack);
}

void crow::travel(crow::packet* pack) {
	gxx::syslock().lock();
	travelled.move_back(*pack);
	gxx::syslock().unlock();
}

void crow::travel_error(crow::packet* pack) {
	crow::utilize(pack);
}

void crow::incoming_handler(crow::packet* pack) {
	switch(pack->header.type) {
		case G1_G0TYPE: crow::incoming_node_packet(pack); break;
		case G1_G3TYPE: 
			if (crow::pubsub_handler) crow::pubsub_handler(pack); 
			else crow::release(pack);
			break;
		default: 
			if (crow::user_type_handler) crow::user_type_handler(pack);
			else crow::release(pack);
	}
}

void crow::do_travel(crow::packet* pack) {
	if (traveling_handler) traveling_handler(pack);
	if (pack->header.stg == pack->header.alen) {
		//Ветка доставленного пакета.
		crow::revert_address(pack);

		if (pack->header.ack) {
			switch(pack->header.type) {
				case G1_ACK_TYPE: utilize_from_outers(pack); break;
				case G1_ACK21_TYPE: utilize_from_outers(pack); send_ack2(pack); break;
				case G1_ACK22_TYPE: qos_release_from_incoming(pack); break;
				default: break;
			}
			crow::utilize(pack);
			return;
		}

		if (pack->ingate) {
			if (pack->header.qos == crow::TargetACK || pack->header.qos == crow::BinaryACK) {
				crow::send_ack(pack);
			}
			if (pack->header.qos == crow::BinaryACK) {
				for (auto& inc : crow::incoming) {
					
					/*GXX_PRINT(inc.header.seqid == pack->header.seqid);
					GXX_PRINT(inc.header.alen == pack->header.alen);
					GXX_PRINT(memcmp(inc.addrptr(), pack->addrptr(), inc.header.alen) == 0);*/

					if (inc.header.seqid == pack->header.seqid && 
						inc.header.alen == pack->header.alen &&
						memcmp(inc.addrptr(), pack->addrptr(), inc.header.alen) == 0) 
					{
						crow::utilize(pack);
						return;
					}
				}
				gxx::system_lock();
				add_to_incoming_list(pack);
				gxx::system_unlock();
			}
			else crow::tower_release(pack);
		}
		//Если пакет отправлен из данного нода, обслуживание не требуется
		else crow::tower_release(pack);

		if (!pack->header.noexec) {
			if (crow::user_incoming_handler) crow::user_incoming_handler(pack);
			else crow::incoming_handler(pack);
		}
		else crow::release(pack);

		return;
	} 
	else {
		if (transit_handler) transit_handler(pack);
		//Ветка транзитного пакета. Логика поиска врат и пересылки.
		crow::gateway* gate = crow::find_target_gateway(pack);
		if (gate == nullptr) { 	
			crow::travel_error(pack);
		}
		else {
			//Здесь пакет штампуется временем отправки и пересылается во врата.
			//Врата должны после пересылки отправить его назад в башню
			//с помощью return_to_tower для контроля качества.
			gate->send(pack);
		}
	}
}

uint16_t __seqcounter = 0;
void crow::transport(crow::packet* pack) {
	pack->header.stg = 0;
	pack->header.ack = 0;
	gxx::syslock().lock();
	pack->header.seqid = __seqcounter++;
	gxx::syslock().unlock();
	crow::travel(pack);
}

//void crow::send(crow::address& addr, const char* data, size_t len, uint8_t type, crow::QoS qos, uint16_t ackquant) {
void crow::send(const void* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type, crow::QoS qos, uint16_t ackquant) {
	crow::packet* pack = crow::create_packet(nullptr, asize, dsize);
	pack->header.type = type;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	//if (addr) 
	memcpy(pack->addrptr(), addr, asize);
	memcpy(pack->dataptr(), data, dsize);
	crow::transport(pack);
}

void crow::send(const void* addr, uint8_t asize, const iovec* vec, size_t veclen, uint8_t type, crow::QoS qos, uint16_t ackquant) {
	size_t dsize = 0;
	const iovec* it = vec;
	const iovec* const eit = vec + veclen;

	for (; it != eit; ++it) {
		dsize += it->iov_len;
	}

	crow::packet* pack = crow::create_packet(nullptr, asize, dsize);
	pack->header.type = type;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	
	memcpy(pack->addrptr(), addr, asize);

	it = vec;
	char* dst = pack->dataptr(); 
	for (; it != eit; ++it) {
		memcpy(dst, it->iov_base, it->iov_len);
		dst += it->iov_len;
	}
	
	crow::transport(pack);
}

void crow::return_to_tower(crow::packet* pack, crow::status sts) {
	gxx::system_lock();
	if (pack->ingate != nullptr) {
		//Пакет был отправлен, и он нездешний. Уничтожить.
		crow::utilize(pack);
	} else {
		//Пакет здешний.
		if (sts != crow::status::Sended || pack->header.qos == WithoutACK) 
			crow::utilize(pack);
		else add_to_outters_list(pack);
	}
	gxx::system_unlock();
}

void crow::print(crow::packet* pack) {
	crow:print_to(*gxx::standart_output, pack);
}

void crow::println(crow::packet* pack) {
	crow:print_to(*gxx::standart_output, pack);
	gxx::print_to(*gxx::standart_output, "\n");
}

void crow::print_to(gxx::io::ostream& out, crow::packet* pack) {
	gxx::fprint_to(out, "("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}, "
		"released:{}"
		")", 
		pack->header.qos,
		(uint8_t)pack->header.ack, 
		pack->header.alen, 
		(uint8_t)pack->header.type, 
		gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(pack->dataptr(), pack->datasize()), 
		pack->flags
	);
}

void crow::revert_address(crow::packet* pack) {
	auto first = pack->addrptr();
	auto last = pack->addrptr() + pack->header.alen;
	while ((first != last) && (first != --last)) {
        char tmp = *last;
        *last = *first;
        *first++ = tmp;
    }
}

void crow::send_ack(crow::packet* pack) {
	auto ack = crow::create_packet(nullptr, pack->header.alen, 0);
	ack->header.type = pack->header.qos == crow::QoS::BinaryACK ? G1_ACK21_TYPE : G1_ACK_TYPE;
	ack->header.ack = 1;
	ack->header.qos = crow::QoS::WithoutACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	ack->released_by_world = true;
	crow::travel(ack);
}

void crow::send_ack2(crow::packet* pack) {
	auto ack = crow::create_packet(nullptr, pack->header.alen, 0);
	ack->header.type = G1_ACK22_TYPE;
	ack->header.ack = 1;
	ack->header.qos = crow::QoS::WithoutACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	crow::travel(ack);
}

void crow::onestep_travel_only() {
	gxx::syslock lock;
	while(1) {
		lock.lock();
		bool empty = crow::travelled.empty();
		if (empty) break;
		crow::packet* pack = &*crow::travelled.begin();
		crow::travelled.unbind(*pack);
		lock.unlock();
		crow::do_travel(pack);
	} 
	lock.unlock();
}

/**
	@todo Переделать очередь пакетов, выстроив их в порядке работы таймеров. 
*/
void crow::onestep() {
	gxx::syslock lock;
	while(1) {
		lock.lock();
		bool empty = crow::travelled.empty();
		if (empty) break;
		crow::packet* pack = &*crow::travelled.begin();
		crow::travelled.unbind(*pack);
		lock.unlock();
		crow::do_travel(pack);
	} 

	uint16_t curtime = crow::millis();
	gxx::for_each_safe(crow::outters.begin(), crow::outters.end(), [&](crow::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
			dlist_del(&pack.lnk);
			if (++pack.ackcount == 5) {
				if (crow::undelivered_handler) crow::undelivered_handler(&pack);
				else crow::utilize(&pack);
			} else {
				crow::travel(&pack);
			}		
		}
	});

	gxx::for_each_safe(crow::incoming.begin(), crow::incoming.end(), [&](crow::packet& pack) {
		if (curtime - pack.last_request_time > pack.header.ackquant) {
			if (++pack.ackcount == 5) {
				crow::utilize(&pack);
			}
			else {
				pack.last_request_time = curtime;
				crow::send_ack(&pack);
			}	
		}
	});
	lock.unlock();
}

void crow::spin() {
	while(1) {
		for (auto& gate : gateways) gate.nonblock_onestep();
		crow::onestep();
	}
}

crow::host::host(const char* addr) {
	int prelen = strlen(addr);
	data = (uint8_t*)malloc(prelen);
	int len = hexer(data, prelen, addr, prelen);
	size = len;
	data = (uint8_t*)realloc(data, len);
}

crow::host::host(const uint8_t* addr, size_t size) {
	this->data = (uint8_t*)malloc(size);
	this->size = size;
	memcpy(data, addr, size);
}

crow::host::host(const host& oth) {
	this->data = (uint8_t*)malloc(oth.size);
	this->size = oth.size;
	memcpy(data, oth.data, size);	
}

crow::host::~host() {
	free(data);
}

void crow::print_dump(crow::packet* pack) {
	gxx::println(gxx::dstring(&pack->header, pack->header.flen));
} 