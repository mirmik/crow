/**
@file
@brief tower file
*/

#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#define DEFAULT_ACKQUANT 200

#include <gxx/container/dlist.h>
#include <crow/gateway.h>
#include <gxx/print.h>
#include <gxx/datastruct/iovec.h>

namespace crow {
	enum class status : uint8_t {
		Sended,
		WrongAddress,
	};

	struct host {
		uint8_t* data;
		size_t size;
		host() = default;
		host(const host& oth);
		host(const uint8_t* addr, size_t size);
		host(const char* str);
		host& operator=(const host& oth) { 
			data = (uint8_t*)malloc(oth.size);
			memcpy(data, oth.data, oth.size);
			size = oth.size;
			return *this;
		}

		bool operator!= (const host& oth) const {
			return size != oth.size || memcmp(data, oth.data, size) != 0;
		}

		bool operator== (const host& oth) const {
			return size == oth.size && memcmp(data, oth.data, size) == 0;
		}

		~host();
		/*bool operator<(const host& oth) const {
			return memcmp(data, oth.data, size) < 0;
		}*/
	};

	///Список врат.
	extern gxx::dlist<crow::gateway, &crow::gateway::lnk> gateways;
	extern gxx::dlist<crow::packet, &crow::packet::lnk> incoming;
	extern gxx::dlist<crow::packet, &crow::packet::lnk> outters;
	extern gxx::dlist<crow::packet, &crow::packet::lnk> travelled;

	///Переместить пакет дальше по конвееру врат.
	void travel_error(crow::packet* pack); 
	void travel(crow::packet* pack); 
	void do_travel(crow::packet* pack); 
	
	void transport(crow::packet* pack); 
	void send(const void* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type = 0, crow::QoS qos = (crow::QoS)0, uint16_t ackquant = 200);
	void send(const void* addr, uint8_t asize, const gxx::iovec* vec, size_t veclen, uint8_t type = 0, crow::QoS qos = (crow::QoS)0, uint16_t ackquant = 200);
	
	///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
	void return_to_tower(crow::packet* pack, status sts);

	///Подключить врата к башне.
	inline void link_gate(crow::gateway* gate, uint8_t id) { 
		//logger.debug("gateway {} added", id);
		gateways.move_back(*gate);
		gate->id = id; 
	} 


	crow::gateway* find_target_gateway(const crow::packet* pack);


	void release(crow::packet* pack);
	void tower_release(crow::packet* pack);
	void print(crow::packet* pack);
	void print_dump(crow::packet* pack);
	void println(crow::packet* pack);
	void print_to(gxx::io::ostream& out, crow::packet* pack);
	void println_to(gxx::io::ostream& out, crow::packet* pack);

	void revert_address(crow::packet* pack);

	void send_ack(crow::packet* pack);
	void send_ack2(crow::packet* pack);

	extern void (*user_type_handler)(crow::packet* pack);
	extern void (*user_incoming_handler)(crow::packet* pack);
	extern void (*traveling_handler)(crow::packet* pack);
	extern void (*transit_handler)(crow::packet* pack);

	/// Обработчик недоставленного пакета. Определяется локальным софтом.
	/// Освобождение должно производиться функцией tower_release.
	extern void(*undelivered_handler)(crow::packet* pack);
	extern void(*pubsub_handler)(crow::packet* pack);

	uint16_t millis();

	void onestep();
	void onestep_travel_only();
	void spin();

	void incoming_handler(crow::packet* pack);
	void incoming_node_packet(crow::packet* pack);
	void incoming_pubsub_packet(crow::packet* pack);
	//void incoming_channel_packet(crow::packet* pack);

	void enable_diagnostic();
}

#endif