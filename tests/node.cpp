#include <crow/address.h>
#include <crow/nodes/node_delegate.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <doctest/doctest.h>
#include <nos/print.h>
#include "allocator_test_helper.h"

#include <chrono>
#include <thread>

static int count = 0;
static int ucount = 0;
static auto addr = crow::address(".99");
static auto waddr = crow::address(".12.127.0.0.1:10998");

void incom(crow::node_packet_ptr)
{
    count++;
}

void undel(crow::node_packet_ptr)
{
    ucount++;
}

TEST_CASE("node0" * doctest::timeout(5))
{
    FOR_EACH_ALLOCATOR
    {
        count = ucount = 0;
        crow::default_tower().reset_total_travelled();
        crow::set_default_incoming_handler(nullptr);

    SUBCASE("0")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(11, addr, "data", 0, 2);

        crow::onestep();

        CHECK_EQ(count, 1);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 2);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("1")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        auto ptr = node0.send(11, addr, "data", 1, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 1);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 4);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 1);
    }

    SUBCASE("2")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(11, addr, "data", 2, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 1);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 6);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("0 wrong node")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(9, addr, "data", 0, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 4); // pack*2 + err(qos:0)*2
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("1 wrong node")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(9, addr, "data", 1, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 6); // pack + err(qos:2)
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("2 wrong node")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(9, addr, "data", 2, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 8); // pack + err(qos:2)
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("0_undel")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(11, waddr, "data", 0, 2);

        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
        crow::onestep();

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 0);
        CHECK_EQ(crow::get_total_travelled(), 1);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("1_undel")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        auto ptr = node0.send(11, waddr, "data", 1, 2);

        // ackquant=2 gets quantized to 4ms, need 5 retries
        // Use 15ms intervals with more iterations to ensure all retries complete
        for (int i = 0; i < 12; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            crow::onestep();
        }

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 1);
        CHECK_EQ(crow::get_total_travelled(), 5);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 1);
    }

    SUBCASE("2_undel")
    {
        crow::node_delegate node0(incom, undel);
        crow::node_delegate node1(incom, undel);

        node0.bind(10);
        node1.bind(11);

        node0.send(11, waddr, "data", 2, 2);

        // ackquant=2 gets quantized to 4ms, need 5 retries
        // Use 15ms intervals with more iterations to ensure all retries complete
        for (int i = 0; i < 12; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            crow::onestep();
        }

        CHECK_EQ(count, 0);
        CHECK_EQ(ucount, 1);
        CHECK_EQ(crow::get_total_travelled(), 5);
        CHECK_EQ(crow::has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }
    } // FOR_EACH_ALLOCATOR
}
