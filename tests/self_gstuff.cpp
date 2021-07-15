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
	char buf[64];

	crow::self_driven_gstuff gate;

	gate.bind(13);

	gate.set_send_buffer(buf);
	gate.set_write_callback(write, NULL);
	auto * pack = crow::create_packet(nullptr, 1, 10);

    pack->header.f.type = 0 & 0x1F;
    pack->header.qos = 0;
    pack->header.ackquant = 0;

    memcpy(pack->addrptr(), "\x01", 1);
    memcpy(pack->dataptr(), "helloworld", 10);

    CHECK_EQ(ccc, 0);
    gate.send(pack);
    CHECK_EQ(ccc, 1);

	crow::release(pack);
}