#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/address.h>
#include <crow/nodes/cli.h>

#include <thread>
#include <chrono>

static int count = 0;
static int ucount = 0;
static auto addr = crow::address(".12.127.0.0.1:10999");
static auto waddr = crow::address(".12.127.0.0.1:10998");

static void incom(crow::node_packet_ptr) 
{
	count++;
}

static void undel(crow::node_packet_ptr) 
{
	ucount++;
}

TEST_CASE("cli" * doctest::timeout(0.5))
{
}