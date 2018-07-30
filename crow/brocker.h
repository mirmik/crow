#ifndef CROW_BROCKER_H
#define CROW_BROCKER_H

#include <crow/pubsub.h>
#include <list>
#include <unordered_set>
#include <string>

namespace crow {
	struct subscriber {
		bool g3;
		crow::host host;
		crow::QoS qos;
		uint16_t ackquant;

		subscriber() = default;
		subscriber(const crow::host& host, bool g3, crow::QoS qos, uint16_t ackquant) : host(host), g3(g3), qos(qos), ackquant(ackquant) {}
		subscriber(const subscriber& oth) : host(oth.host), g3(oth.g3), qos(oth.qos), ackquant(oth.ackquant) {}

		/*bool operator<(const subscriber& oth) const {
			return g3 < oth.g3 && host < oth.host;
		}*/

		bool operator==(const subscriber& oth) const {
			return g3 == oth.g3 && host == oth.host;
		}

		bool operator!=(const subscriber& oth) const {
			return g3 != oth.g3 || host != oth.host;
		}
	};
}


namespace std {
	template<>
	struct hash<crow::subscriber> {
		size_t operator()(const crow::subscriber & x) const noexcept {
     		return (x.g3 * 1234) ^ std::hash<std::string>()(std::string((char*)x.host.data, x.host.size));
     	}
	};
}

namespace crow {
	struct theme {
		theme() = default;
		theme(const std::string& name) : name(name) {}
		theme(const theme& oth) : name(oth.name) {}
		std::string name;
		std::unordered_set<crow::subscriber> subs;
		void publish(const std::string& data);
	};
}

#endif