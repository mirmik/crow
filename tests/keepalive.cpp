#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/proto/node.h>
#include <iostream>
#include <thread>
#include <chrono>

static int a = 0;

class test_keepalive_node : public  crow::node 
{
	void keepalive_handle() override 
	{
		a++;
	}

	void incoming_packet(crow::packet*) override {}
};

TEST_CASE("keepalive") 
{
	test_keepalive_node n;
	n.install_keepalive(10);

	//std::this_thread::sleep_for(std::chrono::microseconds(450));
	int64_t start = millis();
	while(millis() - start < 45) 
	{
		crow::onestep();
	}

	CHECK_EQ(a, 4);
}