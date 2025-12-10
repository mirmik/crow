#ifndef CROW_ALLOCATOR_TEST_HELPER_H
#define CROW_ALLOCATOR_TEST_HELPER_H

#include <crow/packet.h>
#include <crow/tower.h>
#include <doctest/doctest.h>
#include <cstdint>

// Pool buffer for tests (shared across all tests)
inline uint8_t g_test_pool_buffer[512 * 16]; // 16 packets of 512 bytes

struct AllocatorTestGuard
{
    bool use_pool;

    AllocatorTestGuard(bool pool) : use_pool(pool)
    {
        crow::reset_for_test();
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
        crow::reset_for_test();
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

#endif // CROW_ALLOCATOR_TEST_HELPER_H
