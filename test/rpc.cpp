#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/address.h>
#include <crow/proto/rpc.h>

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

int add(int a, int b)
{
	return a + b;
}

TEST_CASE("rpc" * doctest::timeout(0.5))
{
	crow::rpc_node rpc;
	crow::rpc_requestor requestor(addr, 77);

	rpc.add_delegate("add", igris::make_delegate(add));
	rpc.bind(77);

	int ret;
	int sts = requestor.request<int,int,int>("add", ret, 1, 2);

	CHECK_EQ(sts, 0);
	CHECK_EQ(ret, 3);
}