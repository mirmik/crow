#ifndef G0_ECHO_H
#define G0_ECHO_H

#include <crow/node.h>

namespace crow {
	struct echo_node : public node {
		bool quite;

		echo_node(bool quite = false) : quite(quite) {}

		void incoming_packet(crow::packet* pack) override {
			if (!quite) {
				/*gxx::fprintln("echo service incoming_message {0}", gxx::buffer(msg->data, msg->size));
				GXX_PRINT(msg->sid);
				GXX_PRINT(msg->rid);
				GXX_PRINT(msg->pack->header.alen);
				gxx::writeln(msg->data, msg->size);*/				
			}

			auto sh = crow::get_subheader(pack);
			auto ds = crow::get_datasect(pack);

			crow::__node_send(id, sh->sid, pack->addrptr(), pack->header.alen, ds.data(), ds.size(), crow::QoS(0));
			crow::release(pack);
		}
	};

}

#endif