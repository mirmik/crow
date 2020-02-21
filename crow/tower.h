#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#include <crow/gateway.h>
#include <crow/packet_ptr.h>

#include <igris/container/dlist.h>

#define CROW_SENDED 0
#define CROW_WRONG_ADDRESS -1

///Список врат.
extern igris::dlist<crow::gateway, &crow::gateway::lnk> crow_gateways;

namespace crow
{
	extern uint16_t debug_data_size;

	extern dlist_head protocols;
	extern bool diagnostic_noack;

	extern void (*user_incoming_handler)(crow::packet *pack);
	//extern void (*pubsub_handler)(crow::packet *pack);
	//extern void (*node_handler)(crow::packet *pack);
	extern void (*user_type_handler)(crow::packet *pack);
//	extern void (*query_tower_handler)(crow::packet *pack);
//	extern void (*netkeep_tower_handler)(crow::packet *pack);
	extern void (*undelivered_handler)(crow::packet *pack);

	// Передать пакет в обработку.
	packet_ptr travel(crow::packet *pack);
	void nocontrol_travel(crow::packet *pack);

	// Включить трассировку пакетов.
	void diagnostic_enable(); 
	void enable_diagnostic();

	// Включить трассировку аллокации.
	void live_diagnostic_enable();
	void enable_live_diagnostic();

	inline void diagnostic_setup(bool en, bool len=false) 
	{ 
		if (en)
			diagnostic_enable(); 

		if (len)
			live_diagnostic_enable();
	}

	// Отправить пакет.
	crow::packet_ptr send(const void *addr, uint8_t asize,
	          const char *data, uint16_t dsize,
	          uint8_t type, uint8_t qos, uint16_t ackquant);

	crow::packet_ptr send_v(const void *addr, uint8_t asize,
	            const igris::buffer* vec, size_t veclen,
	            uint8_t type, uint8_t qos, uint16_t ackquant);

	static inline crow::packet_ptr send_v(const igris::buffer buf,
	            const igris::buffer* vec, size_t veclen,
	            uint8_t type, uint8_t qos, uint16_t ackquant) 
	{
		return send_v(buf.data(), buf.size(), vec, veclen, type, qos, ackquant);
	}

	crow::packet_ptr send_vv(const igris::buffer buf,
	            const igris::buffer* vec, size_t veclen,
	            const igris::buffer* vec2, size_t veclen2,
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

	[[deprecated]]
	void start_thread();
	
	void start_spin();
	void stop_spin();

	bool has_untravelled();
	void print_list_counts();

	// Завершить операции и потоки.
	void finish();
} // namespace crow

#endif