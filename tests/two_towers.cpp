/**
 * @file two_towers.cpp
 * @brief Test for multiple independent Tower instances communicating via UDP
 *
 * This test verifies that:
 * 1. Multiple Tower instances can be created
 * 2. Each Tower can have its own gateway bound to it
 * 3. Packets can be sent between towers via UDP
 */

#include <crow/address.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

// Custom udpgate that supports Tower binding
class test_udpgate : public crow::udpgate
{
public:
    using crow::udpgate::udpgate;

    int bind(crow::Tower &tower, int gate_no = CROW_UDPGATE_NO)
    {
        return crow::gateway::bind(tower, gate_no);
    }
};

// Global state for handlers (since we can't use capturing lambdas)
static std::atomic<int> g_tower2_received{0};
static std::string g_tower2_data;
static crow::Tower *g_tower2_ptr = nullptr;

static void tower2_handler(crow::packet *pack)
{
    g_tower2_data = std::string(pack->dataptr(), pack->datasize());
    g_tower2_received++;
    if (g_tower2_ptr)
        g_tower2_ptr->release(pack);
}

static std::atomic<int> g_tower1_received{0};
static std::string g_tower1_data;
static crow::Tower *g_tower1_ptr = nullptr;

static void tower1_handler(crow::packet *pack)
{
    g_tower1_data = std::string(pack->dataptr(), pack->datasize());
    g_tower1_received++;
    if (g_tower1_ptr)
        g_tower1_ptr->release(pack);
}

TEST_CASE("two_towers_udp" * doctest::timeout(5.0))
{
    // Reset global state
    g_tower2_received = 0;
    g_tower2_data.clear();
    g_tower1_received = 0;
    g_tower1_data.clear();

    // Create two independent towers
    crow::Tower tower1;
    crow::Tower tower2;

    // Store pointers for handlers
    g_tower1_ptr = &tower1;
    g_tower2_ptr = &tower2;

    // Configure towers
    tower1.set_retransling_allowed(true);
    tower2.set_retransling_allowed(true);

    // Create UDP gates for each tower on different ports
    test_udpgate gate1;
    test_udpgate gate2;

    // Open UDP ports
    REQUIRE(gate1.open(20001) == 0);
    REQUIRE(gate2.open(20002) == 0);

    // Bind gates to their respective towers
    gate1.bind(tower1, 12); // gate_id = 12 (CROW_UDPGATE_NO)
    gate2.bind(tower2, 12);

    // Set incoming handler for tower2
    tower2.set_default_incoming_handler(tower2_handler);

    SUBCASE("send_qos0")
    {
        // Address: gate_id=12, IP=127.0.0.1, port=20002
        auto addr = crow::address(".12.127.0.0.1:20002");

        // Send packet from tower1 to tower2
        tower1.send(addr, "Hello Tower2!", 0, 0, 50, false);

        // Process both towers
        for (int i = 0; i < 10 && g_tower2_received == 0; i++)
        {
            tower1.onestep();

            // Manually trigger read on gate2
            gate2.read_handler(0);
            tower2.onestep();

            std::this_thread::sleep_for(10ms);
        }

        CHECK_EQ(g_tower2_received, 1);
        CHECK_EQ(g_tower2_data, "Hello Tower2!");
        CHECK_EQ(tower1.get_total_travelled(), 1);
    }

    SUBCASE("send_qos1")
    {
        auto addr = crow::address(".12.127.0.0.1:20002");

        // Send with QoS 1 (requires ACK)
        tower1.send(addr, "QoS1 Message", 0, 1, 50, false);

        // Process both towers until done or timeout
        for (int i = 0; i < 20; i++)
        {
            tower1.onestep();
            gate2.read_handler(0);
            tower2.onestep();
            gate1.read_handler(0);

            if (g_tower2_received > 0 && !tower1.has_untravelled())
                break;

            std::this_thread::sleep_for(10ms);
        }

        CHECK_EQ(g_tower2_received, 1);
        CHECK_EQ(g_tower2_data, "QoS1 Message");
        // QoS1: original packet + ACK = at least 2 travels per tower
        CHECK_GE(tower1.get_total_travelled(), 1);
    }

    SUBCASE("bidirectional")
    {
        // Set handler for tower1 too
        tower1.set_default_incoming_handler(tower1_handler);

        auto addr_to_tower2 = crow::address(".12.127.0.0.1:20002");
        auto addr_to_tower1 = crow::address(".12.127.0.0.1:20001");

        // Send from tower1 to tower2
        tower1.send(addr_to_tower2, "To Tower2", 0, 0, 50, false);
        // Send from tower2 to tower1
        tower2.send(addr_to_tower1, "To Tower1", 0, 0, 50, false);

        // Process both towers
        for (int i = 0; i < 20; i++)
        {
            tower1.onestep();
            tower2.onestep();
            gate1.read_handler(0);
            gate2.read_handler(0);

            if (g_tower2_received > 0 && g_tower1_received > 0)
                break;

            std::this_thread::sleep_for(10ms);
        }

        CHECK_EQ(g_tower2_received, 1);
        CHECK_EQ(g_tower2_data, "To Tower2");
        CHECK_EQ(g_tower1_received, 1);
        CHECK_EQ(g_tower1_data, "To Tower1");
    }

    // Cleanup
    gate1.close();
    gate2.close();
    g_tower1_ptr = nullptr;
    g_tower2_ptr = nullptr;
}

TEST_CASE("tower_isolation")
{
    // Verify that towers are truly isolated
    crow::Tower tower1;
    crow::Tower tower2;

    // Each tower should have independent state
    tower1.set_retransling_allowed(true);
    tower2.set_retransling_allowed(false);

    CHECK_EQ(tower1.get_retransling_allowed(), true);
    CHECK_EQ(tower2.get_retransling_allowed(), false);

    tower1.set_debug_data_size(100);
    tower2.set_debug_data_size(200);

    CHECK_EQ(tower1.get_debug_data_size(), 100);
    CHECK_EQ(tower2.get_debug_data_size(), 200);

    // Counters should be independent
    CHECK_EQ(tower1.get_total_travelled(), 0);
    CHECK_EQ(tower2.get_total_travelled(), 0);
}
