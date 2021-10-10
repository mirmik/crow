#include <doctest/doctest.h>
#include <crow/gates/self_driven_gstuff.h>

#include <thread>
#include <chrono>

static volatile int ccc = 0;

static
int write(void * priv, const char * data, unsigned int size)
{
	ccc = 1;
	return size;
}

TEST_CASE("self_driven_gstuff")
{
	crow::total_travelled = 0;
	crow::user_incoming_handler = nullptr;

	{
		char buf[64];

		crow::self_driven_gstuff gate;

		gate.init(buf, write, NULL);
		gate.bind(13);

		auto * pack = crow_create_packet(nullptr, 1, 10);

		pack->header.u.f.type = 0 & 0x1F;
		pack->header.qos = 0;
		pack->header.ackquant = 0;

		memcpy(crow_packet_addrptr(pack), "\x01", 1);
		memcpy(crow_packet_dataptr(pack), "helloworld", 10);

		CHECK_EQ(ccc, 0);
		gate.send(pack);
		CHECK_EQ(ccc, 1);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		crow::release(pack);
	}

	CHECK_EQ(crow::total_travelled, 0);
	CHECK_EQ(crow::has_untravelled(), false);
	CHECK_EQ(crow_allocated_count, 0);
}

TEST_CASE("self_driven_gstuff2")
{
	crow::total_travelled = 0;
	crow::user_incoming_handler = nullptr;
	CHECK_EQ(crow::has_allocated(), false);

	{
		char buf[64];
		char buf2[64];

		crow::self_driven_gstuff gate;

		gate.init(buf, write, NULL);
		gate.bind(13);

		auto * pack = crow_create_packet(nullptr, 1, 10);

		pack->header.u.f.type = 9 & 0x1F;
		pack->header.qos = 0;
		pack->header.ackquant = 0;

		memcpy(crow_packet_addrptr(pack), "\x01", 1);
		memcpy(crow_packet_dataptr(pack), "helloworld", 10);

		int gsize = gstuffing((const char*)&pack->header, pack->header.flen, buf2);

		for (int i = 0; i < gsize; ++i)
		{
			gate.newdata(buf2[i]);
		}

		crow::onestep();

		crow::utilize(pack);
	}

	CHECK_EQ(crow::total_travelled, 1);
	CHECK_EQ(crow::has_untravelled(), false);
	CHECK_EQ(crow_allocated_count, 0);
}