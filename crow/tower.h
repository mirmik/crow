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

	extern void (*unsleep_handler)();

	// Передать пакет в обработку.
	packet_ptr travel(crow::packet *pack);
	void nocontrol_travel(crow::packet *pack);

	// Включить трассировку пакетов.
	void diagnostic_enable();
	void enable_diagnostic();

	// Включить трассировку аллокации.
	void live_diagnostic_enable();
	void enable_live_diagnostic();

	void diagnostic_setup(bool en, bool len = false);
	
	// Отправить пакет.
	crow::packet_ptr send(const crow::hostaddr & addr,
	                      igris::buffer data,
	                      uint8_t type,
	                      uint8_t qos,
	                      uint16_t ackquant,
	                      bool fastsend=false);

	crow::packet_ptr send_v(const crow::hostaddr & addr,
	                        const igris::buffer* vec,
	                        size_t veclen,
	                        uint8_t type,
	                        uint8_t qos,
	                        uint16_t ackquant,
	                        bool fastsend=false);

	crow::packet_ptr send_vv(const crow::hostaddr & addr,
	                         const igris::buffer* vec, size_t veclen,
	                         const igris::buffer* vec2, size_t veclen2,
	                         uint8_t type,
	                         uint8_t qos,
	                         uint16_t ackquant,
	                         bool fastsend=false);

	// Эта функция вызывается вратами после обработки отсылаемого пакета.
	void return_to_tower(crow::packet *pack, uint8_t sts);

	// Присоеденить врата к башне.
	static inline int link_gate(crow::gateway *gate, uint8_t id)
	{
		gate->id = id;

		for (auto & gate : crow_gateways)
		{
			if (gate.id == id)
				return -1;
		}

		crow_gateways.add_last(*gate);
		return 0;
	}

	// Используется пользовательским кодом для освобождения пакета.
	void release(crow::packet *pack);

	// Взятие отметки времени.
	uint16_t millis();

	void onestep();
	void onestep_travel_only();
	void spin();
	void spin_with_select();

	void stop_spin();
	void start_spin_with_select();
	void start_spin_without_select();

	void start_spin();

	[[deprecated]]
	void spin_join();
	void join_spin();

	bool has_untravelled();
	bool has_untravelled_now();
	void print_list_counts();

	bool fully_empty();

	int64_t get_minimal_timeout();

	// Завершить гейты.
	void finish();
} // namespace crow

#endif
