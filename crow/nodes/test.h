#ifndef G0_TEST_H
#define G0_TEST_H

#include <crow/tower.h>
#include <igris/print.h>
#include <igris/print/stdprint.h>
#include <igris/util/string.h>

namespace crow
{
	struct test_node : public node
	{
		void incoming_packet(crow::packet *pack) override
		{
			auto sh = crow::get_subheader(pack);
			auto data = igris::buffer(pack->dataptr(), pack->datasize());
			crow::diagnostic("here", pack);
			igris::fprintln("subheader: sid={}, rid={}", (uint16_t)sh->sid,
							(uint16_t)sh->rid);
			igris::fprintln("datasect: {}", igris::dstring(data));
			crow::release(pack);
		}
	};

	static inline crow::test_node *create_test_node(int i)
	{
		test_node *n = new test_node;
		crow::link_node(n, i);
		return n;
	}
} // namespace crow

#endif