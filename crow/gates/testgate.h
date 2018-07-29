#ifndef G1_TESTGATE_H
#define G1_TESTGATE_H

#include <crow/tower.h>

namespace crow {
	struct testgate : public gateway {
		void send(crow::packet* pack) override {
			gxx::println("TestGate send");
			gxx::fprintln("TestGate: {0}", gxx::buffer(pack->dataptr(), pack->datasize()));
			crow::return_to_tower(pack, crow::status::Sended);
		}
	};
}

#endif