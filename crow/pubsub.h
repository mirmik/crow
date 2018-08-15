/**
	@file
*/

#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <crow/tower.h>
#include <crow/host.h>

struct crow_theme;

#define SUBSCRIBE 	0
#define PUBLISH 	1
#define MESSAGE 	2

typedef struct crow_subheader_pubsub {
	uint8_t type;
	uint8_t thmsz;
} G1_PACKED crow_subheader_pubsub_t;

typedef struct subheader_pubsub_data {
	uint8_t datsz;
} G1_PACKED crow_subheader_pubsub_data_t;

typedef struct subheader_pubsub_control {
	uint8_t qos;
	uint16_t ackquant;
} G1_PACKED crow_subheader_pubsub_control_t;

__BEGIN_DECLS

static inline crow_subheader_pubsub_t* get_subheader_pubsub(crow_packet_t* pack) {
	return (crow_subheader_pubsub_t*) crow_packet_dataptr(pack);
}

static inline crow_subheader_pubsub_data_t* get_subheader_pubsub_data(crow_packet_t* pack) {
	return (crow_subheader_pubsub_data_t*) (crow_packet_dataptr(pack) + sizeof(crow_subheader_pubsub_t));
}

static inline crow_subheader_pubsub_control_t* get_subheader_pubsub_control(crow_packet_t* pack) {
	return (crow_subheader_pubsub_control_t*) (crow_packet_dataptr(pack) + sizeof(crow_subheader_pubsub_t));
}

//gxx::buffer pubsub_message_datasect(crow_packet_t* pack);

/*static inline gxx::buffer pubsub_theme(crow_packet_t* pack) {
	crow_subheader_pubsub_t* shps = crow::get_subheader_pubsub(pack);
	crow_subheader_pubsub_t* shps_d = crow::get_subheader_pubsub_data(pack);
	return gxx::buffer(crow_packet_dataptr(pack) + sizeof(crow_subheader_pubsub_t) + sizeof(crow_subheader_pubsub_data_t), shps->thmsz);
}

static inline gxx::buffer pubsub_data(crow_packet_t* pack) {
	crow_subheader_pubsub_t* shps = crow::get_subheader_pubsub(pack);
	crow_subheader_pubsub_t* shps_d = crow::get_subheader_pubsub_data(pack);
	return gxx::buffer(crow_packet_dataptr(pack) + sizeof(crow_subheader_pubsub_t) + sizeof(crow_subheader_pubsub_data_t) + shps->thmsz, shps_d->datsz);		
}*/

void crow_publish_buffer(const char* theme, const void* data, size_t datsz);
void crow_publish(const char* theme, const char* data);
//void crow_subscribe(const void* theme, size_t thmsz, uint8_t qos);
void crow_subscribe(const char* theme, uint8_t qos);

//void crow_set_publish_host(const crow::host& brocker_host);
//void crow_set_publish_qos(crow::QoS qos);

__END_DECLS

#endif