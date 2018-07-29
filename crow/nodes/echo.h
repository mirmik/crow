#ifndef G0_ECHO_H
#define G0_ECHO_H

#include <g0/service.h>
#include <g0/core.h>

namespace g0 {

	struct echo_service : public basic_service {
		bool quite;
		echo_service(bool quite = false) : quite(quite) {}
		void incoming_message(g0::message* msg) override {
			if (!quite) {
				gxx::fprintln("echo service incoming_message {0}", gxx::buffer(msg->data, msg->size));
				GXX_PRINT(msg->sid);
				GXX_PRINT(msg->rid);
				GXX_PRINT(msg->pack->header.alen);
				gxx::writeln(msg->data, msg->size);
				
			}
			


			if (msg->pack == nullptr) g0::send(id, msg->sid, msg->data, msg->size);
			else g0::send(id, msg->sid, msg->pack->addrptr(), msg->pack->header.alen, msg->data, msg->size, g1::QoS(0));
			g0::utilize(msg);
		}
	};

}

#endif