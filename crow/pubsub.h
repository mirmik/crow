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

		uint8_t thm_size;

		union {
			//PUBLISH
			struct {
				uint16_t dat_size;
			};

			//SUBSCRIBE
			struct {
				crow::QoS qos;
				uint16_t ackquant;
			};
		};
	};

	struct subscribe {
		theme* 		tptr;

		union {
			struct { ///< g3 
				crow::QoS qos;
			};

			struct { ///< g2 socket
								
			};
		};
	};

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