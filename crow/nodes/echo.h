#ifndef G0_ECHO_H
#define G0_ECHO_H

#include <crow/node.h>
#include <crow/tower.h>

namespace crow {
	struct echo_node : public node {
		bool quite;

		echo_node(bool quite = false) : quite(quite) {}

		void incoming_packet(crow::packet* pack) override {
			if (!quite) {
				/*owl::fprintln("echo service incoming_message {0}", owl::buffer(msg->data, msg->size));
				owl_PRINT(msg->sid);
				owl_PRINT(msg->rid);
				owl_PRINT(msg->pack->header.alen);
				owl::writeln(msg->data, msg->size);*/				
			}

			auto sh = crow::get_subheader(pack);
			auto ds = owl::buffer(pack->dataaddr(), pack->datasize());

			crow::node_send(id, sh->sid, pack->addrptr(), pack->header.alen, ds.data(), ds.size(), crow::QoS(0));
			crow::release(pack);
		}
	};

}

#endif