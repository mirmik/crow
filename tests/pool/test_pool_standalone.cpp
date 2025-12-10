/**
 * @file test_pool_standalone.cpp
 * @brief Standalone tests for crow packet pool allocator
 *
 * Compile with:
 * g++ -std=c++17 -g -I.. -I../../igris test_pool_standalone.cpp \
 *     ../crow/src/variants/allocation_pool.cpp ../crow/src/packet.cpp \
 *     -o test_pool_standalone
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <crow/packet.h>
#include <cstring>

// Stubs for system_lock/unlock when testing on host without OS
extern "C" void system_lock() {}
extern "C" void system_unlock() {}

TEST_CASE("pool_allocator.init_and_basic")
{
    // Size: packet + header_v1 + 256 bytes data
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    const int num_packets = 4;

    static uint8_t pool_buffer[elem_size * num_packets];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    CHECK_EQ(crow::allocated_count(), 0);
    CHECK_EQ(crow::get_package_pool()->avail(), num_packets);
    CHECK_EQ(crow::get_package_pool()->size(), num_packets);
}

TEST_CASE("pool_allocator.allocate_v1")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    static uint8_t pool_buffer[elem_size * 4];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *pack = crow::allocate_packet_header_v1(256);

    CHECK(pack != nullptr);
    CHECK_EQ(crow::allocated_count(), 1);
    CHECK(pack->header_addr() != nullptr);
    CHECK(pack->addrptr() != nullptr);

    crow::deallocate_packet(pack);
    CHECK_EQ(crow::allocated_count(), 0);
}

TEST_CASE("pool_allocator.allocate_v0")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v0) + 256;
    static uint8_t pool_buffer[elem_size * 4];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *pack = crow::allocate_packet_header_v0(256);

    CHECK(pack != nullptr);
    CHECK_EQ(crow::allocated_count(), 1);

    crow::deallocate_packet(pack);
    CHECK_EQ(crow::allocated_count(), 0);
}

TEST_CASE("pool_allocator.allocate_with_sizes")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    static uint8_t pool_buffer[elem_size * 4];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *pack = crow::allocate_packet_header_v1(10, 100);

    CHECK(pack != nullptr);
    CHECK_EQ(pack->addrsize(), 10);
    CHECK_EQ(pack->datasize(), 100);

    crow::deallocate_packet(pack);
}

TEST_CASE("pool_allocator.exhaust_pool")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 128;
    const int num_packets = 3;
    static uint8_t pool_buffer[elem_size * num_packets];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *p1 = crow::allocate_packet_header_v1(128);
    crow::packet *p2 = crow::allocate_packet_header_v1(128);
    crow::packet *p3 = crow::allocate_packet_header_v1(128);

    CHECK(p1 != nullptr);
    CHECK(p2 != nullptr);
    CHECK(p3 != nullptr);
    CHECK_EQ(crow::allocated_count(), 3);

    // Pool exhausted
    crow::packet *p4 = crow::allocate_packet_header_v1(128);
    CHECK(p4 == nullptr);

    // Free one and try again
    crow::deallocate_packet(p1);
    CHECK_EQ(crow::allocated_count(), 2);

    crow::packet *p5 = crow::allocate_packet_header_v1(128);
    CHECK(p5 != nullptr);
    CHECK_EQ(crow::allocated_count(), 3);

    // Cleanup
    crow::deallocate_packet(p2);
    crow::deallocate_packet(p3);
    crow::deallocate_packet(p5);
    CHECK_EQ(crow::allocated_count(), 0);
}

TEST_CASE("pool_allocator.data_integrity")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    static uint8_t pool_buffer[elem_size * 2];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *pack = crow::allocate_packet_header_v1(16, 64);
    CHECK(pack != nullptr);

    // Write to address field
    memset(pack->addrptr(), 0xAA, 16);
    // Write to data field
    memset(pack->dataptr(), 0xBB, 64);

    // Verify
    for (int i = 0; i < 16; i++)
        CHECK_EQ(pack->addrptr()[i], 0xAA);
    for (int i = 0; i < 64; i++)
        CHECK_EQ(pack->dataptr()[i], (char)0xBB);

    crow::deallocate_packet(pack);
}

TEST_CASE("pool_allocator.packet_fields")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    static uint8_t pool_buffer[elem_size * 2];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    crow::packet *pack = crow::allocate_packet_header_v1(8, 32);
    CHECK(pack != nullptr);

    // Set packet fields
    pack->set_type(3);
    pack->set_quality(1);
    pack->set_ackquant(504);  // quantized to 50*10+4=504
    pack->set_seqid(0x1234);
    pack->set_stage(2);

    // Verify fields
    CHECK_EQ(pack->type(), 3);
    CHECK_EQ(pack->quality(), 1);
    CHECK_EQ(pack->ackquant(), 504);  // ackquant is quantized
    CHECK_EQ(pack->seqid(), 0x1234);
    CHECK_EQ(pack->stage(), 2);

    crow::deallocate_packet(pack);
}

TEST_CASE("pool_allocator.has_allocated")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 128;
    static uint8_t pool_buffer[elem_size * 2];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    CHECK_FALSE(crow::has_allocated());

    crow::packet *p1 = crow::allocate_packet_header_v1(128);
    CHECK(crow::has_allocated());

    crow::packet *p2 = crow::allocate_packet_header_v1(128);
    CHECK(crow::has_allocated());

    crow::deallocate_packet(p1);
    CHECK(crow::has_allocated()); // p2 still allocated

    crow::deallocate_packet(p2);
    CHECK_FALSE(crow::has_allocated());
}

TEST_CASE("pool_allocator.multiple_pools")
{
    // Test reinitializing pool with different buffer
    const size_t elem_size1 =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 64;
    static uint8_t pool_buffer1[elem_size1 * 2];
    crow::engage_packet_pool(pool_buffer1, sizeof(pool_buffer1), elem_size1);

    CHECK_EQ(crow::get_package_pool()->size(), 2);

    crow::packet *p1 = crow::allocate_packet_header_v1(64);
    CHECK(p1 != nullptr);
    crow::deallocate_packet(p1);

    // Reinitialize with larger pool
    const size_t elem_size2 =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 128;
    static uint8_t pool_buffer2[elem_size2 * 4];
    crow::engage_packet_pool(pool_buffer2, sizeof(pool_buffer2), elem_size2);

    CHECK_EQ(crow::get_package_pool()->size(), 4);

    crow::packet *p2 = crow::allocate_packet_header_v1(128);
    CHECK(p2 != nullptr);
    crow::deallocate_packet(p2);
}

TEST_CASE("pool_allocator.deallocate_nullptr")
{
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 64;
    static uint8_t pool_buffer[elem_size * 2];
    crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

    // Should not crash
    crow::deallocate_packet(nullptr);
    CHECK_EQ(crow::allocated_count(), 0);
}
