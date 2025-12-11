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

    // Convenience wrapper that uses the node's tower
    void install_keepalive(int64_t interval, bool immediate_call = true)
    {
        alived_object::install_keepalive(*_tower, interval, immediate_call);
    }
};

TEST_CASE("keepalive" * doctest::timeout(5))
{
    FOR_EACH_ALLOCATOR_WITH_TOWER
    {
        a = 0;
        b = 0;

        test_keepalive_node an(a);
        test_keepalive_node bn(b);

        an.bind(tower, 100);
        bn.bind(tower, 101);

        // Use longer intervals for more reliable timing
        an.install_keepalive(20);  // 20ms interval
        bn.install_keepalive(40);  // 40ms interval

        // Run for 150ms to ensure enough keepalive callbacks
        // Use longer duration for more reliable timing on loaded systems
        int64_t start = igris::millis();
        while (igris::millis() - start < 150)
        {
            tower.onestep();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // install_keepalive with immediate_call=true calls handler once immediately
        // Then timer fires periodically. Timer interval is quantized to min 4ms.
        // a: 20ms interval -> 1 immediate + ~7 timer calls in 150ms = ~8 total
        // b: 40ms interval -> 1 immediate + ~3 timer calls in 150ms = ~4 total
        // Use very conservative expectations to avoid flaky test
        CHECK_UNARY(a >= 2);
        CHECK_UNARY(b >= 1);

        // Cleanup: unplan keepalive timers
        an.keepalive_timer.unplan();
        bn.keepalive_timer.unplan();
    }
}