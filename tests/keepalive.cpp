#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/proto/node.h>
#include <iostream>
#include <thread>
#include <chrono>

static int a = 0;
static int b = 0;

class test_keepalive_node : public  crow::node, public crow::alived_object 
{
	int& ptr;
public:
	test_keepalive_node(int& ptr) : ptr(ptr) {} 

	void keepalive_handle() override 
	{
		ptr++;
	}

	void incoming_packet(crow::packet*) override {}
};

TEST_CASE("keepalive") 
{
	test_keepalive_node an(a);
	test_keepalive_node bn(b);
	an.install_keepalive(10);
	bn.install_keepalive(20);

	int64_t start = millis();
	while(millis() - start < 45) 
	{
		crow::onestep();
	}

	CHECK_EQ(a, 5);
	CHECK_EQ(b, 3);
}