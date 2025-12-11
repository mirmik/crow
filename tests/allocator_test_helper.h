#ifndef CROW_ALLOCATOR_TEST_HELPER_H
#define CROW_ALLOCATOR_TEST_HELPER_H

#include <crow/packet.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/gates/loopgate.h>
#include <crow/gates/udpgate.h>
#include <doctest/doctest.h>
#include <cstdint>

// Pool buffer for tests (shared across all tests)
inline uint8_t g_test_pool_buffer[512 * 16]; // 16 packets of 512 bytes

struct AllocatorTestGuard
{
    bool use_pool;

    AllocatorTestGuard(bool pool) : use_pool(pool)
    {
        if (pool)
        {
            crow::engage_packet_pool(g_test_pool_buffer,
                                     sizeof(g_test_pool_buffer), 512);
        }
    }

    ~AllocatorTestGuard()
    {
        if (use_pool)
        {
            crow::disengage_packet_pool();
        }
    }
};

/// Guard that creates a local Tower with loopgate for isolated testing.
/// Usage:
///   TowerTestGuard guard(use_pool);
///   crow::Tower &tower = guard.tower;
///   // Use tower.send(), tower.onestep(), etc.
struct TowerTestGuard
{
    bool use_pool;
    crow::Tower tower;
    crow::loopgate loopgate;

    TowerTestGuard(bool pool) : use_pool(pool)
    {
        tower.reset_for_test();
        if (pool)
        {
            crow::engage_packet_pool(g_test_pool_buffer,
                                     sizeof(g_test_pool_buffer), 512);
        }
        tower.set_retransling_allowed(true);
        loopgate.bind(tower, 99);
    }

    ~TowerTestGuard()
    {
        if (use_pool)
        {
            crow::disengage_packet_pool();
        }
        tower.reset_for_test();
    }
};

/// Guard that creates a local Tower with loopgate and udpgate for testing.
/// Usage:
///   TowerTestGuardWithUdp guard(use_pool, port);
///   crow::Tower &tower = guard.tower;
struct TowerTestGuardWithUdp
{
    bool use_pool;
    crow::Tower tower;
    crow::loopgate loopgate;
    crow::udpgate udpgate;

    TowerTestGuardWithUdp(bool pool, uint16_t udp_port = 10099) : use_pool(pool)
    {
        tower.reset_for_test();
        if (pool)
        {
            crow::engage_packet_pool(g_test_pool_buffer,
                                     sizeof(g_test_pool_buffer), 512);
        }
        tower.set_retransling_allowed(true);
        loopgate.bind(tower, 99);
        udpgate.bind(tower, 12);
        udpgate.open(udp_port);
    }

    ~TowerTestGuardWithUdp()
    {
        udpgate.close();
        if (use_pool)
        {
            crow::disengage_packet_pool();
        }
        tower.reset_for_test();
    }
};

// Macro to run test body with both allocators
// Usage:
//   TEST_CASE("my_test") {
//       FOR_EACH_ALLOCATOR {
//           // test code runs twice: with malloc and with pool
//       }
//   }
#define FOR_EACH_ALLOCATOR                                                     \
    for (bool _use_pool : {false, true})                                       \
        if (auto _subcase = doctest::detail::Subcase(                          \
                _use_pool ? "pool" : "malloc", __FILE__, __LINE__))            \
            if (AllocatorTestGuard _guard(_use_pool); true)

// Macro to run test with local Tower and loopgate for each allocator
// Usage:
//   TEST_CASE("my_test") {
//       FOR_EACH_ALLOCATOR_WITH_TOWER {
//           tower.send(...);
//           tower.onestep();
//       }
//   }
#define FOR_EACH_ALLOCATOR_WITH_TOWER                                          \
    for (bool _use_pool : {false, true})                                       \
        if (auto _subcase = doctest::detail::Subcase(                          \
                _use_pool ? "pool" : "malloc", __FILE__, __LINE__))            \
            if (TowerTestGuard _guard(_use_pool); true)                        \
                if (crow::Tower &tower = _guard.tower; true)

#endif // CROW_ALLOCATOR_TEST_HELPER_H
