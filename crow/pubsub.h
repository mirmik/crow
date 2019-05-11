/**
	@file
*/

#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <assert.h>
#include <crow/tower.h>
#include <igris/buffer.h>

#include <string>

struct crow_theme;

#define SUBSCRIBE 0
#define PUBLISH 1
#define MESSAGE 2

typedef struct crow_subheader_pubsub
{
	uint8_t type;
	uint8_t thmsz;
} G1_PACKED crow_subheader_pubsub_t;

typedef struct crow_subheader_pubsub_data
{
	uint16_t datsz;
} G1_PACKED crow_subheader_pubsub_data_t;

typedef struct crow_subheader_pubsub_control
{
	uint8_t qos;
	uint16_t ackquant;
} G1_PACKED crow_subheader_pubsub_control_t;

namespace crow
{
	void publish(const char *theme, const char *data, uint8_t qos = 0,
				 uint16_t acktime = DEFAULT_ACKQUANT);
	// void publish(const char* theme, const std::string& data, uint8_t qos=0,
	// uint16_t acktime=DEFAULT_ACKQUANT);
	void subscribe(const char *theme, uint8_t qos = 0,
				   uint16_t acktime = DEFAULT_ACKQUANT, uint8_t rqos = 0,
				   uint16_t racktime = DEFAULT_ACKQUANT);

	void publish_buffer(const char *theme, const void *data, uint16_t datsz,
						uint8_t qos, uint16_t acktime);
	void set_publish_host(const uint8_t *hhost, size_t hsize);

	void set_crowker(const std::string &str);
	std::string envcrowker();
	std::string environment_crowker();

	static inline crow_subheader_pubsub_t *
	get_subheader_pubsub(crow::packet *pack)
	{
		return (crow_subheader_pubsub_t *)pack->dataptr();
	}

	static inline crow_subheader_pubsub_data_t *
	get_subheader_pubsub_data(crow::packet *pack)
	{
		return (
			crow_subheader_pubsub_data_t *)(pack->dataptr() +
											sizeof(crow_subheader_pubsub_t));
	}

	static inline crow_subheader_pubsub_control_t *
	get_subheader_pubsub_control(crow::packet *pack)
	{
		return (
			crow_subheader_pubsub_control_t *)(pack->dataptr() +
											   sizeof(crow_subheader_pubsub_t));
	}

	static inline char *packet_pubsub_thmptr(struct crow::packet *pack)
	{
		return pack->dataptr() + sizeof(crow_subheader_pubsub_t) +
			   sizeof(crow_subheader_pubsub_data_t);
	}

	static inline char *packet_pubsub_datptr(struct crow::packet *pack)
	{
		return crow::packet_pubsub_thmptr(pack) +
			   get_subheader_pubsub(pack)->thmsz;
	}

	namespace pubsub
	{
		static inline igris::buffer get_theme(crow::packet *pack)
		{
			assert(pack->header.f.type == CROW_PUBSUB_PROTOCOL);

			struct crow_subheader_pubsub *shps = get_subheader_pubsub(pack);
			// struct crow_subheader_pubsub_data * shps_d =
			// get_subheader_pubsub_data(pack);

			return igris::buffer(crow::packet_pubsub_thmptr(pack), shps->thmsz);
		}

		static inline igris::buffer get_data(crow::packet *pack)
		{
			assert(pack->header.f.type == CROW_PUBSUB_PROTOCOL);

			// struct crow_subheader_pubsub * shps = get_subheader_pubsub(pack);
			struct crow_subheader_pubsub_data *shps_d =
				get_subheader_pubsub_data(pack);

			return igris::buffer(crow::packet_pubsub_datptr(pack),
								 shps_d->datsz);
		}
	} // namespace pubsub
} // namespace crow

#endif