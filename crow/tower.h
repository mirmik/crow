/**
@file
@brief tower file
*/

#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#include <crow/gateway.h>
#include <sys/uio.h>

#define CROW_SENDED 0
#define CROW_WRONG_ADDRESS -1

///Список врат.
extern struct dlist_head crow_gateways;
//extern dlist_head incoming;
//extern dlist_head outters;
//extern dlist_head travelled;

__BEGIN_DECLS

///Переместить пакет дальше по конвееру врат.
void crow_travel_error(crow_packet_t* pack); 
void crow_travel(crow_packet_t* pack); 
void crow_do_travel(crow_packet_t* pack); 
	
void crow_transport(crow_packet_t* pack); 
void crow_send(const void* addr, uint8_t asize, const char* data, uint16_t dsize, uint8_t type, uint8_t qos, uint16_t ackquant);
void crow_send_v(const void* addr, uint8_t asize, const struct iovec* vec, size_t veclen, uint8_t type, uint8_t qos, uint16_t ackquant);
	
///Вызывается на только что отправленный пакет. Башня или уничтожает его, или кеширует для контроля качества.
void crow_return_to_tower(crow_packet_t* pack, uint8_t sts);

///Подключить врата к башне.
static inline void crow_link_gate(struct crow_gw* gate, uint8_t id) { 
	gate->id = id; 
	dlist_move_back(&crow_gateways, &gate->lnk);
} 


struct crow_gw* crow_find_target_gateway(const crow_packet_t* pack);

void crow_release(crow_packet_t* pack);
void crow_tower_release(crow_packet_t* pack);

void crow_print(crow_packet_t* pack);
void crow_println(crow_packet_t* pack);
void crow_print_dump(crow_packet_t* pack);

void crow_revert_address(crow_packet_t* pack);

void crow_send_ack(crow_packet_t* pack);
void crow_send_ack2(crow_packet_t* pack);

extern void (*crow_user_type_handler)(crow_packet_t* pack);
extern void (*crow_user_incoming_handler)(crow_packet_t* pack);
extern void (*crow_traveling_handler)(crow_packet_t* pack);
extern void (*crow_transit_handler)(crow_packet_t* pack);

/// Обработчик недоставленного пакета. Определяется локальным софтом.
/// Освобождение должно производиться функцией tower_release.
extern void(*crow_undelivered_handler)(crow_packet_t* pack);
extern void(*crow_pubsub_handler)(crow_packet_t* pack);

uint16_t crow_millis();

void crow_onestep();
void crow_onestep_travel_only();
void crow_spin();

void crow_incoming_handler(crow_packet_t* pack);
void crow_incoming_node_packet(crow_packet_t* pack);
void crow_incoming_pubsub_packet(crow_packet_t* pack);
//void incoming_channel_packet(crow_packet_t* pack);

void crow_enable_diagnostic();

__END_DECLS

#endif