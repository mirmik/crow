#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/address.h>
#include <crow/nodes/rshell_cli_node.h>
#include <crow/nodes/node_delegate.h>
#include <nos/print.h>

#include <thread>
#include <chrono>

static int count = 0;
static int count2 = 0;
static auto addr = crow::address(".99");

static void incom_test(crow::node_packet_ptr ptr)
{
	auto data = ptr.message();

	if (strncmp(data.data(), "HelloWorld", 10) == 0)
	{
		count2++;
	}
}

static void undel_test(crow::node_packet_ptr)
{}

void cli_handle(char * buf, int len, char* ans, int maxanslen)
{
	CHECK_EQ(len, 5);

	if (strncmp(buf, "hello", 5) == 0)
	{
		count++;
	}

	snprintf(ans, maxanslen, "HelloWorld");
}

TEST_CASE("rshell_cli" * doctest::timeout(0.5))
{
	crow::rshell_cli_node_delegate rnode(igris::make_delegate(cli_handle));
	crow::node_delegate node0(incom_test, undel_test);

	rnode.bind(10);
	node0.bind(11);

	CHECK_EQ(count, 0);

	node0.send(10, addr, "hello", 0, 2);

	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();
	std::this_thread::sleep_for(std::chrono::milliseconds(3));
	crow::onestep();

	CHECK_EQ(count, 1);
	CHECK_EQ(count2, 1);
}