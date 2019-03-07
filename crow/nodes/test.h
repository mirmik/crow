#ifndef G0_TEST_H
#define G0_TEST_H

#include <owl/util/string.h>
#include <owl/print.h>
#include <owl/print/stdprint.h>
#include <crow/tower.h>

namespace crow {
	struct test_node : public node {
		void incoming_packet(crow::packet* pack) override {
			auto sh = crow::get_subheader(pack);
			auto data = owl::buffer(pack->dataptr(), pack->datasize());
			crow::diagnostic("here", pack);
			owl::fprintln("subheader: sid={}, rid={}", (uint16_t)sh->sid, (uint16_t)sh->rid);
			owl::fprintln("datasect: {}", owl::dstring(data));
			crow::release(pack);
		}
	};

	static inline crow::test_node* create_test_node(int i) 
	{
		test_node* n = new test_node;
		crow::link_node(n, i);
		return n;
	}
}

#endif