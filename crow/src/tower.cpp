/**
@file tower.cpp
*/

#define NOTRACE 1

#include <stdbool.h>
#include <crow/tower.h>
#include <gxx/syslock.h>

#include <gxx/util/hexer.h>
#include <gxx/debug/dprint.h>

#include <assert.h>
#include <string.h>

#include <gxx/trace.h>

/*gxx::dlist<crow::gateway, &crow::gateway::lnk> crow::gateways;
gxx::dlist<crow::packet, &crow::packet::lnk> crow_travelled;
gxx::dlist<crow::packet, &crow::packet::lnk> crow_incoming;
gxx::dlist<crow::packet, &crow::packet::lnk> crow_outters;*/

gxx::dlist<crow::gateway, &crow::gateway::lnk> crow_gateways;

DLIST_HEAD(crow_travelled);
DLIST_HEAD(crow_incoming);
DLIST_HEAD(crow_outters);

void(*crow::pubsub_handler)(crow::packet* pack) = nullptr;
void(*crow::node_handler)(crow::packet* pack) = nullptr;
void(*crow::user_type_handler)(crow::packet* pack) = nullptr;
void(*crow::user_incoming_handler)(crow::packet* pack) = nullptr;
void(*crow::undelivered_handler)(crow::packet* pack) = nullptr;

static bool __diagnostic_enabled = false;
static bool __live_diagnostic_enabled = false;

void crow::diagnostic_enable()
{
	TRACE();
	__diagnostic_enabled = true;
}

void crow::live_diagnostic_enable()
{
	TRACE();
	__live_diagnostic_enabled = true;
}

void crow::enable_diagnostic()
{
	TRACE();
	__diagnostic_enabled = true;
}

void crow::enable_live_diagnostic()
{
	TRACE();
	__live_diagnostic_enabled = true;
}

static void crow_utilize(crow::packet* pack)
{
	TRACE();
	if (__live_diagnostic_enabled)
		crow::diagnostic("utilz", pack);

	system_lock();
	dlist_del(&pack->lnk); // Очищается в tower_release((см. tower.c))
	dlist_del(&pack->ulnk);
	crow::deallocate_packet(pack);
	system_unlock();
}

static crow::gateway* crow_find_target_gateway(crow::packet* pack)
{
	TRACE();
	uint8_t gidx = *pack->stageptr();

	for (crow::gateway& gate : crow_gateways)
	{
		if (gate.id == gidx)
			return &gate;
	}

	return NULL;
}

void crow::release(crow::packet* pack)
{
	TRACE();
	system_lock();

	if (pack->f.released_by_tower) crow_utilize(pack);
	else pack->f.released_by_world = true;

	system_unlock();
}

static void crow_tower_release(crow::packet* pack)
{
	TRACE();
	system_lock();

	//Скорее всего это поле должно освобождаться без инициализации.
	//Инициализируем для последуещего освобождения в utilize
	dlist_del_init(&pack->lnk);

	if (pack->f.released_by_world) crow_utilize(pack);
	else pack->f.released_by_tower = true;

	system_unlock();
}

static void utilize_from_outers(crow::packet* pack)
{
	TRACE();
	crow::packet* it;
	dlist_for_each_entry(it, &crow_outters, lnk)
	{
		if (
		    it->header.seqid == pack->header.seqid &&
		    pack->header.alen == it->header.alen &&
		    !memcmp(it->addrptr(), pack->addrptr(), pack->header.alen)
		)
		{
			crow_utilize(it);
			return;
		}
	}
}

static void qos_release_from_incoming(crow::packet* pack)
{
	TRACE();
	crow::packet* it;
	dlist_for_each_entry(it, &crow_incoming, lnk)
	{
		if (it->header.seqid == pack->header.seqid &&
		        pack->header.alen == it->header.alen &&
		        !memcmp(it->addrptr(), pack->addrptr(), pack->header.alen)
		   )
		{
			crow_tower_release(it);
			return;
		}
	}
}

static void add_to_incoming_list(crow::packet* pack)
{	
	TRACE();
	pack->last_request_time = crow::millis();
	dlist_move_tail(&pack->lnk, &crow_incoming);
}

static void add_to_outters_list(crow::packet* pack)
{
	TRACE();
	pack->last_request_time = crow::millis();
	dlist_move_tail(&pack->lnk, &crow_outters);
}

void crow::travel(crow::packet* pack)
{
	TRACE();
	system_lock();
	dlist_add_tail(&pack->lnk, &crow_travelled);
	system_unlock();
}

static void crow_travel_error(crow::packet* pack)
{
	TRACE();
	crow_utilize(pack);
}

static void crow_incoming_handler(crow::packet* pack)
{
	TRACE();
	switch (pack->header.f.type)
	{
		case G1_G0TYPE:
			if (crow::node_handler) crow::node_handler(pack);
			else crow::release(pack);

			break;

		case G1_G3TYPE:
			if (crow::pubsub_handler) crow::pubsub_handler(pack);
			else crow::release(pack);

			break;

		default:
			if (crow::user_type_handler) crow::user_type_handler(pack);
			else crow::release(pack);
	}
}

static void crow_send_ack(crow::packet* pack)
{
	TRACE();
	crow::packet* ack = crow::create_packet(NULL, pack->header.alen, 0);
	ack->header.f.type = pack->header.qos == CROW_BINARY_ACK ? G1_ACK21_TYPE : G1_ACK_TYPE;
	ack->header.f.ack = 1;
	ack->header.qos = CROW_WITHOUT_ACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	ack->f.released_by_world = true;
	crow::travel(ack);
}

static void crow_send_ack2(crow::packet* pack)
{
	TRACE();
	crow::packet* ack = crow::create_packet(NULL, pack->header.alen, 0);
	ack->header.f.type = G1_ACK22_TYPE;
	ack->header.f.ack = 1;
	ack->header.qos = CROW_WITHOUT_ACK;
	ack->header.seqid = pack->header.seqid;
	memcpy(ack->addrptr(), pack->addrptr(), pack->header.alen);
	crow::travel(ack);
}

static void crow_revert_address(crow::packet* pack)
{
	TRACE();
	uint8_t* first = pack->addrptr();
	uint8_t* last = pack->addrptr() + pack->header.alen;

	while ((first != last) && (first != --last))
	{
		char tmp = *last;
		*last = *first;
		*first++ = tmp;
	}
}

static void crow_do_travel(crow::packet* pack)
{
	TRACE();
	//if (crow_traveling_handler)
	// crow_traveling_handler(pack);

	if (pack->header.stg == pack->header.alen)
	{
		//Ветка доставленного пакета.
		crow_revert_address(pack);

		if (pack->header.f.ack)
		{
			//Перехватываем ack пакеты.
			if (__diagnostic_enabled)
				crow::diagnostic("inack", pack);

			switch (pack->header.f.type)
			{
				case G1_ACK_TYPE:
					utilize_from_outers(pack);
					break;

				case G1_ACK21_TYPE:
					utilize_from_outers(pack);
					crow_send_ack2(pack);
					break;

				case G1_ACK22_TYPE:
					qos_release_from_incoming(pack);
					break;

				default:
					break;
			}

			crow_utilize(pack);
			return;
		}

		if (__diagnostic_enabled)
			crow::diagnostic("incom", pack);

		if (pack->ingate)
		{
			//Если пакет пришел извне, используем логику обеспечения качества.

			//Для пакетов с подтверждение посылаем ack первого или второго типов.
			if (pack->header.qos == CROW_TARGET_ACK || pack->header.qos == CROW_BINARY_ACK)
				crow_send_ack(pack);

			if (pack->header.qos == CROW_BINARY_ACK)
			{
				//Перед тем как добавить пакет в обработку, проверяем,
				//нет ли его в списке принятых.
				crow::packet* inc;
				dlist_for_each_entry(inc, &crow_incoming, lnk)
				{
					if (inc->header.seqid == pack->header.seqid &&
					        inc->header.alen == pack->header.alen &&
					        memcmp(inc->addrptr(), pack->addrptr(), inc->header.alen) == 0)
					{
						crow_utilize(pack);
						return;
					}
				}
				system_lock();
				add_to_incoming_list(pack);
				system_unlock();
			}
			else
			{
				crow_tower_release(pack);
			}
		}
		else
		{
			//Если пакет отправлен из данного нода, обслуживание не требуется
			crow_tower_release(pack);
		}

		//Решаем, что делать с пришедшим пакетом.
		if (!pack->header.f.noexec)
		{
			if (crow::user_incoming_handler) crow::user_incoming_handler(pack);
			else crow_incoming_handler(pack);
		}
		else crow::release(pack);

		return;
	}
	else
	{
		//if (crow_transit_handler) crow_transit_handler(pack);
		//Ветка транзитного пакета. Логика поиска врат и пересылки.
		crow::gateway* gate = crow_find_target_gateway(pack);

		if (gate == NULL)
		{
			if (__diagnostic_enabled)
				crow::diagnostic("wgate", pack);

			crow_travel_error(pack);
		}
		else
		{
			if (__diagnostic_enabled)
				crow::diagnostic("trans", pack);

			//Здесь пакет штампуется временем отправки и пересылается во врата.
			//Врата должны после пересылки отправить его назад в башню
			//с помощью return_to_tower для контроля качества.
			gate->send(pack);
		}
	}
}

uint16_t __seqcounter = 0;
static void crow_transport(crow::packet* pack)
{
	TRACE();
	pack->header.stg = 0;
	pack->header.f.ack = 0;
	system_lock();
	pack->header.seqid = __seqcounter++;
	system_unlock();
	crow::travel(pack);
}

//void crow_send(crow_address& addr, const char* data, size_t len, uint8_t type, uint8_t qos, uint16_t ackquant) {
void crow::send(const void* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type, uint8_t qos, uint16_t ackquant)
{
	TRACE();
	crow::packet* pack = crow::create_packet(NULL, asize, dsize);
	pack->header.f.type = type & 0x1F;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	//if (addr)
	memcpy(pack->addrptr(), addr, asize);
	memcpy(pack->dataptr(), data, dsize);
	crow_transport(pack);
}

void crow::send_v(const void* addr, uint8_t asize, const struct iovec* vec, size_t veclen, uint8_t type, uint8_t qos, uint16_t ackquant)
{
	TRACE();
	size_t dsize = 0;
	const struct iovec* it = vec;
	const struct iovec* const eit = vec + veclen;

	for (; it != eit; ++it)
	{
		dsize += it->iov_len;
	}

	crow::packet* pack = crow::create_packet(NULL, asize, dsize);
	pack->header.f.type = type & 0x1F;
	pack->header.qos = qos;
	pack->header.ackquant = ackquant;
	//if (addr)
	memcpy(pack->addrptr(), addr, asize);

	it = vec;
	char* dst = pack->dataptr();

	for (; it != eit; ++it)
	{
		memcpy(dst, it->iov_base, it->iov_len);
		dst += it->iov_len;
	}

	crow_transport(pack);
}

void crow::return_to_tower(crow::packet* pack, uint8_t sts)
{
	TRACE();
	system_lock();

	if (pack->ingate != NULL)
	{
		//Пакет был отправлен, и он нездешний. Уничтожить.
		crow_utilize(pack);
	}
	else
	{
		//Пакет здешний.
		if (sts != CROW_SENDED || pack->header.qos == CROW_WITHOUT_ACK)
			crow_utilize(pack);
		else add_to_outters_list(pack);
	}

	system_unlock();
}

void crow::onestep_travel_only()
{
	TRACE();
	system_lock();

	while (1)
	{
		system_lock();
		bool empty = dlist_empty(&crow_travelled);

		if (empty) break;

		crow::packet* pack = dlist_first_entry(&crow_travelled, crow::packet, lnk);
		dlist_del_init(&pack->lnk);
		system_unlock();
		crow_do_travel(pack);
	}

	system_unlock();
}

static inline void crow_onestep_send_stage() 
{
	crow::packet* pack;

	TRACE();

	while (1)
	{
		system_lock();

		if (dlist_empty(&crow_travelled))
		{
			break;
		}

		pack = dlist_first_entry(&crow_travelled, crow::packet, lnk);
		dlist_del_init(&pack->lnk);
		system_unlock();
		crow_do_travel(pack);
		
		system_unlock();
	}
	system_unlock();
}

static inline void crow_onestep_outers_stage()
{
	crow::packet* pack;
	crow::packet* n;

	TRACE();

	uint16_t curtime = crow::millis();

	system_lock();
	if (dlist_empty(&crow_outters))	
	{
		system_unlock();
		return;
	}
	system_unlock();	


	system_lock();
	dlist_for_each_entry_safe(pack, n, &crow_outters, lnk)
	{
		assert(pack->f.released_by_tower == 0);

		if (curtime - pack->last_request_time > pack->header.ackquant)
		{
			dlist_del_init(&pack->lnk);

			if (++pack->ackcount == 5)
			{
				crow_tower_release(pack);

				if (crow::undelivered_handler) crow::undelivered_handler(pack);
				else crow::release(pack);
			}
			else
			{
				crow::travel(pack);
			}
		}
	}
	system_unlock();
} 

static inline void crow_onestep_incoming_stage()
{
	crow::packet* pack;
	crow::packet* n;
	
	TRACE();

	uint16_t curtime = crow::millis();

	system_lock();
	if (dlist_empty(&crow_incoming))	
	{
		system_unlock();
		return;
	}
	system_unlock();	

	system_lock();
	dlist_for_each_entry_safe(pack, n, &crow_incoming, lnk)
	{
		assert(pack->f.released_by_tower == 0);

		if (curtime - pack->last_request_time > pack->header.ackquant)
		{
			if (++pack->ackcount == 5)
			{
				crow_utilize(pack);
			}
			else
			{
				pack->last_request_time = curtime;
				crow_send_ack(pack);
			}
		}
	}
	system_unlock();
} 

/**
	@todo Переделать очередь пакетов, выстроив их в порядке работы таймеров.
*/
void crow::onestep()
{
	TRACE();

	//crow::gateway* gate;
	for (crow::gateway& gate : crow_gateways)
	{
		gate.nblock_onestep();
	}
	
	crow_onestep_send_stage();
	crow_onestep_outers_stage();
	crow_onestep_incoming_stage();
}

void crow::spin()
{
	TRACE();
	while (1)
	{
		crow::onestep();
	}
}