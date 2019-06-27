#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#include <crow/gateway.h>
#include <sys/uio.h>

#include <igris/container/dlist.h>

#define CROW_SENDED 0
#define CROW_WRONG_ADDRESS -1

///Список врат.
extern igris::dlist<crow::gateway, &crow::gateway::lnk> crow_gateways;

namespace crow
{
	extern dlist_head protocols;

	extern void (*user_incoming_handler)(crow::packet *pack);
	//extern void (*pubsub_handler)(crow::packet *pack);
	//extern void (*node_handler)(crow::packet *pack);
	extern void (*user_type_handler)(crow::packet *pack);
//	extern void (*query_tower_handler)(crow::packet *pack);
//	extern void (*netkeep_tower_handler)(crow::packet *pack);
	extern void (*undelivered_handler)(crow::packet *pack);

	// Передать пакет в обработку.
	void travel(crow::packet *pack);

	// Включить трассировку пакетов.
	void diagnostic_enable();
	void enable_diagnostic();

	// Включить трассировку аллокации.
	void live_diagnostic_enable();
	void enable_live_diagnostic();

	// Отправить пакет.
	void send(const void *addr, uint8_t asize,
	          const char *data, uint16_t dsize,
	          uint8_t type, uint8_t qos, uint16_t ackquant);

	void send_v(const void *addr, uint8_t asize,
	            const struct iovec *vec, size_t veclen,
	            uint8_t type, uint8_t qos, uint16_t ackquant);

	// Эта функция вызывается вратами после обработки отсылаемого пакета.
	void return_to_tower(crow::packet *pack, uint8_t sts);

	// Присоеденить врата к башне.
	static inline void link_gate(struct crow::gateway *gate, uint8_t id)
	{
		gate->id = id;
		crow_gateways.add_last(*gate);
	}

	// Используется пользовательским кодом для освобождения пакета.
	void release(crow::packet *pack);

	// Взятие отметки времени.
	uint16_t millis();

	void onestep();
	void onestep_travel_only();
	void spin();

	void start_thread();

	bool has_untravelled();
} // namespace crow

#endif