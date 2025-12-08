#include <chrono>
#include <crow/proto/node.h>
#include <crow/tower.h>
#include <doctest/doctest.h>
#include <iostream>
#include <thread>
#include "allocator_test_helper.h"

static int a = 0;
static int b = 0;

class test_keepalive_node : public crow::node, public crow::alived_object
{
    int &ptr;

public:
    test_keepalive_node(int &ptr) : ptr(ptr) {}

    void keepalive_handle() override
    {
        ptr++;
    }

    void incoming_packet(crow::packet *) override {}
};

TEST_CASE("keepalive")
{
    FOR_EACH_ALLOCATOR
    {
        a = 0;
        b = 0;

        test_keepalive_node an(a);
        test_keepalive_node bn(b);
        an.install_keepalive(10);
        bn.install_keepalive(20);

        // Run for 60ms to ensure enough keepalive callbacks
        int64_t start = igris::millis();
        while (igris::millis() - start < 60)
        {
            crow::onestep();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // a: 10ms interval -> 6+ calls in 60ms (including immediate)
        // b: 20ms interval -> 3+ calls in 60ms (including immediate)
        CHECK_UNARY(a >= 4);
        CHECK_UNARY(b >= 2);

        // Cleanup: unplan keepalive timers
        an.keepalive_timer.unplan();
        bn.keepalive_timer.unplan();
    }
}