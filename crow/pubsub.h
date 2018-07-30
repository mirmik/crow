#ifndef CROW_PUBSUB_H
#define CROW_PUBSUB_H

#include <crow/tower.h>
#include <unordered_map>

namespace crow {
	struct theme;

	enum class frame_type {
		SUBSCRIBE = 0,
		PUBLISH = 1,
		MESSAGE = 2,
	};

	struct subheader_pubsub {
		frame_type type;
		uint8_t thmsz;
	} G1_PACKED;

	struct subheader_pubsub_data {
		uint8_t datsz;
	} G1_PACKED;

	struct subheader_pubsub_control {
		crow::QoS qos;
		uint16_t ackquant;
	} G1_PACKED;


	static inline subheader_pubsub* get_subheader_pubsub(crow::packet* pack) {
		return (subheader_pubsub*) pack->dataptr();
	}

	static inline subheader_pubsub_data* get_subheader_pubsub_data(crow::packet* pack) {
		return (subheader_pubsub_data*) (pack->dataptr() + sizeof(subheader_pubsub));
	}

	static inline subheader_pubsub_control* get_subheader_pubsub_control(crow::packet* pack) {
		return (subheader_pubsub_control*) (pack->dataptr() + sizeof(subheader_pubsub));
	}

	static inline gxx::buffer pubsub_theme(crow::packet* pack) {
		auto shps = crow::get_subheader_pubsub(pack);
		auto shps_d = crow::get_subheader_pubsub_data(pack);
		return gxx::buffer(pack->dataptr() + sizeof(subheader_pubsub) + sizeof(subheader_pubsub_data), shps->thmsz);
	}

	static inline gxx::buffer pubsub_data(crow::packet* pack) {
		auto shps = crow::get_subheader_pubsub(pack);
		auto shps_d = crow::get_subheader_pubsub_data(pack);
		return gxx::buffer(pack->dataptr() + sizeof(subheader_pubsub) + sizeof(subheader_pubsub_data) + shps->thmsz, shps_d->datsz);		
	}

	/*struct subscribe {
		theme* 		tptr;

		union {
			struct { ///< g3 
				crow::QoS qos;
			};

			struct { ///< g2 socket
								
			};
		};
	};*/

	void publish(const char* theme, size_t thmsz, const char* data, size_t datsz);
	void subscribe(const char* theme, size_t thmsz, crow::QoS qos = crow::QoS(0), uint16_t ackquant = DEFAULT_ACKQUANT);


	extern void(*subscribe_handler)(crow::packet* pack);
	
	//extern crow::host brocker_host;
	void set_publish_host(const crow::host& brocker_host);
	void set_publish_qos(crow::QoS qos);

	/*struct subscriber {
		std::unordered_map<std::string, subscribe> themes;	

		uint8_t* 	raddr_ptr;
		size_t 		raddr_len;
	
		uint8_t 	proto;

		
	};*/

	/*struct theme {
		std::unordered_map<std::string, subscriber*> subscribers;
	};

	void incoming(g1::packet* pack);
	void incoming_dataframe(const char* data, size_t size);*/


}

#endif