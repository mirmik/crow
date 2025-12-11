#include <crow/address.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <doctest/doctest.h>
#include <nos/print.h>
#include "allocator_test_helper.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

static auto addr = crow::address(".99");
static auto addr2 = crow::address(".99.99");
static int count = 0;
static crow::Tower *g_tower = nullptr;

void incoming(crow::packet *ptr)
{
    count++;
    crow::Tower::release(ptr);
}

TEST_CASE("ptr")
{
    FOR_EACH_ALLOCATOR
    {
        {
            auto ptr = crow::create_packet(nullptr, 0, 0);
            crow::packet_ptr pptr(ptr);
            // tower_release invalidates the packet immediately, regardless of refs
            // pptr destructor will handle the packet_ptr cleanup
            crow::Tower::tower_release(ptr);
        }
        CHECK_EQ(crow::allocated_count(), 0);
    }
}

TEST_CASE("get_stage")
{
    FOR_EACH_ALLOCATOR_WITH_TOWER
    {
        std::string data = "data";
        crow::packet *pack =
            crow::create_packet(NULL, addr.size(), data.size());
        pack->set_type(5);
        pack->set_quality(0);
        pack->set_ackquant(0);
        memcpy(pack->addrptr(), addr.data(), addr.size());
        memcpy(pack->dataptr(), data.data(), data.size());
        CHECK_NE(pack, nullptr);
        CHECK_EQ(pack->stage(), 0);
        CHECK_EQ(*pack->stageptr(), 99);
        pack->invalidate();
        CHECK_EQ(crow::allocated_count(), 0);
    }
}

TEST_CASE("get_stage_2")
{
    static auto waddr = crow::address(".12.127.0.0.1:10098");
    for (bool _use_pool : {false, true})
    {
        auto _subcase = doctest::detail::Subcase(
            _use_pool ? "pool" : "malloc", __FILE__, __LINE__);
        if (!_subcase)
            continue;

        TowerTestGuardWithUdp _guard(_use_pool, 10098);
        std::string data = "data";
        crow::packet *pack =
            crow::create_packet(NULL, waddr.size(), data.size());
        pack->set_type(5);
        pack->set_quality(0);
        pack->set_ackquant(0);
        memcpy(pack->addrptr(), waddr.data(), waddr.size());
        memcpy(pack->dataptr(), data.data(), data.size());
        CHECK_NE(pack, nullptr);
        CHECK_EQ(pack->stage(), 0);
        CHECK_EQ(*pack->stageptr(), 12);
        pack->invalidate();
        CHECK_EQ(crow::allocated_count(), 0);
    }
}

TEST_CASE("test0" * doctest::timeout(3.0))
{
    FOR_EACH_ALLOCATOR_WITH_TOWER
    {
        count = 0;
        tower.set_default_incoming_handler(incoming);
        CHECK_EQ(crow::allocated_count(), 0);

    SUBCASE("0")
    {
        tower.send(addr, "data", 0, 0, 20);
        tower.onestep();

        CHECK_EQ(count, 1);
        CHECK_EQ(tower.get_total_travelled(), 2);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("2_safe")
    {
        auto packptr = tower.send(addr, "data", 0, 0, 20);
        CHECK_EQ(packptr->refs, 1);

        tower.onestep();
        std::this_thread::sleep_for(10ms);
        CHECK_EQ(packptr->refs, 1);

        CHECK_EQ(count, 1);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 1);
    }

    SUBCASE("3_safe")
    {
        auto packptr = tower.send(addr, "data0", 0, 0, 20);
        packptr = tower.send(addr, "data1", 0, 0, 20);
        packptr = tower.send(addr, "data2", 0, 0, 20);
        CHECK_EQ(packptr->refs, 1);

        tower.onestep();
        std::this_thread::sleep_for(10ms);
        CHECK_EQ(packptr->refs, 1);

        CHECK_EQ(count, 3);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 1);
    }

    SUBCASE("addr2")
    {
        tower.send(addr2, "data", 0, 0, 20);

        tower.onestep();
        std::this_thread::sleep_for(10ms);

        CHECK_EQ(count, 1);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("qos1")
    {
        tower.send(addr, "data", 0, 1, 20);

        tower.onestep();

        CHECK_EQ(count, 1);
        CHECK_EQ(tower.get_total_travelled(), 4); // pack * 2 + ack * 2
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("qos2")
    {
        tower.send(addr, "data", 0, 2, 20);

        tower.onestep();
        std::this_thread::sleep_for(10ms);

        CHECK_EQ(count, 1);
        CHECK_EQ(tower.get_total_travelled(), 6); // pack * 2 + ack * 2 + ack2 * 2
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }
    } // FOR_EACH_ALLOCATOR_WITH_TOWER
}

// Separate test case for undelivered tests that require UDP gate
TEST_CASE("test0_undelivered" * doctest::timeout(3.0))
{
    static auto waddr = crow::address(".12.127.0.0.1:10098");
    for (bool _use_pool : {false, true})
    {
        auto _subcase = doctest::detail::Subcase(
            _use_pool ? "pool" : "malloc", __FILE__, __LINE__);
        if (!_subcase)
            continue;

        TowerTestGuardWithUdp _guard(_use_pool, 10099);
        crow::Tower &tower = _guard.tower;
        count = 0;
        tower.set_default_incoming_handler(incoming);
        CHECK_EQ(crow::allocated_count(), 0);

    SUBCASE("undelivered_0")
    {
        tower.send(waddr, "data", 0, 0, 2);

        tower.onestep();

        CHECK_EQ(count, 0);
        CHECK_EQ(tower.get_total_travelled(), 1);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("undelivered_1")
    {
        tower.send(waddr, "data", 0, 1, 2);

        CHECK_EQ(tower.incomming_stage_count(), 0);
        CHECK_EQ(tower.outers_stage_count(), 1);
        tower.onestep();

        CHECK_EQ(tower.incomming_stage_count(), 0);

        // ackquant=2 gets quantized to 4ms, need 5 retries
        // Use 15ms intervals with more iterations to ensure all retries complete
        for (int i = 0; i < 12; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            tower.onestep();
        }

        CHECK_EQ(tower.outers_stage_count(), 0);
        CHECK_EQ(count, 0);
        CHECK_EQ(tower.get_total_travelled(), 5);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }

    SUBCASE("undelivered_2")
    {
        tower.send(waddr, "data", 0, 2, 2);

        // ackquant=2 gets quantized to 4ms, need 5 retries
        // Use 15ms intervals with more iterations to ensure all retries complete
        for (int i = 0; i < 12; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            tower.onestep();
        }

        CHECK_EQ(count, 0);
        CHECK_EQ(tower.get_total_travelled(), 5);
        CHECK_EQ(tower.has_untravelled(), false);
        CHECK_EQ(crow::allocated_count(), 0);
    }
    } // for _use_pool
}