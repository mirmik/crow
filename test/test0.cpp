#include <doctest/doctest.h>
#include <crow/tower.h>
#include <crow/address.h>

auto addr = crow::address(".12.127.0.0.1:10999");
auto addr2 = crow::address(".12.127.0.0.1:10999.12.127.0.0.1:10999");

int count = 0;
void incoming(crow::packet * ptr)
{
	count++;
	crow::release(ptr);
}

TEST_CASE("test0" * doctest::timeout(0.5))
{
	count = 0;
	crow::user_incoming_handler = incoming;

	SUBCASE("0")
	{
		crow::send(addr, "data", 0, 0, 20, false);

		while (count != 1);

		CHECK_EQ(count, 1);
		CHECK_EQ(crow::has_untravelled(), false);
		CHECK_EQ(crow::allocated_count, 0);
	}

	SUBCASE("1")
	{
		crow::send(addr, "data", 0, 0, 20, false);

		while (count != 1);

		CHECK_EQ(count, 1);
		CHECK_EQ(crow::has_untravelled(), false);
		CHECK_EQ(crow::allocated_count, 0);
	}

	SUBCASE("2_safe")
	{
		auto packptr = crow::send(addr, "data", 0, 0, 20, false);
		CHECK_EQ(packptr->refs, 1);
	
		while (count != 1);
		CHECK_EQ(packptr->refs, 1);

		CHECK_EQ(count, 1);
		CHECK_EQ(crow::has_untravelled(), false);
		CHECK_EQ(crow::allocated_count, 1);
	}

	SUBCASE("3_safe")
	{
		auto packptr = crow::send(addr, "data0", 0, 0, 20, false);
		packptr = crow::send(addr, "data1", 0, 0, 20, false);
		packptr = crow::send(addr, "data2", 0, 0, 20, false);
		CHECK_EQ(packptr->refs, 1);
	
		while (count != 3);
		CHECK_EQ(packptr->refs, 1);

		CHECK_EQ(count, 3);
		CHECK_EQ(crow::has_untravelled(), false);
		CHECK_EQ(crow::allocated_count, 1);
	}

	SUBCASE("addr2")
	{
		crow::send(addr2, "data", 0, 0, 20, false);

		while (count != 1);

		CHECK_EQ(count, 1);
		CHECK_EQ(crow::has_untravelled(), false);
		CHECK_EQ(crow::allocated_count, 0);
	}
}