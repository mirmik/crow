/**
 * @file allocation_pool.cpp
 * @brief Tests for pool allocator mode
 */

#include <crow/packet.h>
#include <crow/tower.h>
#include <doctest/doctest.h>
#include <cstring>

TEST_CASE("pool_allocator")
{
    crow::reset_for_test();

    // Size: packet + header_v1 + 256 bytes data
    const size_t elem_size =
        sizeof(crow::packet) + sizeof(crow::header_v1) + 256;
    const int num_packets = 8;

    static uint8_t pool_buffer[elem_size * num_packets];

    SUBCASE("init_and_basic")
    {
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);
        CHECK(crow::is_pool_engaged());
        CHECK_EQ(crow::allocated_count(), 0);
        CHECK_EQ(crow::get_package_pool()->avail(), num_packets);
        CHECK_EQ(crow::get_package_pool()->size(), num_packets);
        crow::disengage_packet_pool();
    }

    SUBCASE("allocate_v1")
    {
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

        crow::packet *pack = crow::allocate_packet_header_v1(256);

        CHECK(pack != nullptr);
        CHECK_EQ(crow::allocated_count(), 1);
        CHECK(pack->header_addr() != nullptr);
        CHECK(pack->addrptr() != nullptr);

        crow::deallocate_packet(pack);
        CHECK_EQ(crow::allocated_count(), 0);
        crow::disengage_packet_pool();
    }

    SUBCASE("allocate_v0")
    {
        const size_t elem_size_v0 =
            sizeof(crow::packet) + sizeof(crow::header_v0) + 256;
        static uint8_t pool_buffer_v0[elem_size_v0 * 4];
        crow::engage_packet_pool(pool_buffer_v0, sizeof(pool_buffer_v0),
                                 elem_size_v0);

        crow::packet *pack = crow::allocate_packet_header_v0(256);

        CHECK(pack != nullptr);
        CHECK_EQ(crow::allocated_count(), 1);

        crow::deallocate_packet(pack);
        CHECK_EQ(crow::allocated_count(), 0);
        crow::disengage_packet_pool();
    }

    SUBCASE("allocate_with_sizes")
    {
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

        crow::packet *pack = crow::allocate_packet_header_v1(10, 100);

        CHECK(pack != nullptr);
        CHECK_EQ(pack->addrsize(), 10);
        CHECK_EQ(pack->datasize(), 100);

        crow::deallocate_packet(pack);
        crow::disengage_packet_pool();
    }

    SUBCASE("exhaust_pool")
    {
        const size_t small_elem =
            sizeof(crow::packet) + sizeof(crow::header_v1) + 128;
        const int small_count = 3;
        static uint8_t small_buffer[small_elem * small_count];
        crow::engage_packet_pool(small_buffer, sizeof(small_buffer), small_elem);

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
        crow::disengage_packet_pool();
    }

    SUBCASE("data_integrity")
    {
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

        crow::packet *pack = crow::allocate_packet_header_v1(16, 64);
        CHECK(pack != nullptr);

        // Write to address field
        memset(pack->addrptr(), 0xAA, 16);
        // Write to data field
        memset(pack->dataptr(), 0xBB, 64);

        // Verify
        for (int i = 0; i < 16; i++)
            CHECK_EQ((uint8_t)pack->addrptr()[i], 0xAA);
        for (int i = 0; i < 64; i++)
            CHECK_EQ((uint8_t)pack->dataptr()[i], 0xBB);

        crow::deallocate_packet(pack);
        crow::disengage_packet_pool();
    }

    SUBCASE("packet_fields")
    {
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);

        crow::packet *pack = crow::allocate_packet_header_v1(8, 32);
        CHECK(pack != nullptr);

        // Set packet fields
        pack->set_type(3);
        pack->set_quality(1);
        pack->set_ackquant(504);
        pack->set_seqid(0x1234);
        pack->set_stage(2);

        // Verify fields
        CHECK_EQ(pack->type(), 3);
        CHECK_EQ(pack->quality(), 1);
        CHECK_EQ(pack->ackquant(), 504);
        CHECK_EQ(pack->seqid(), 0x1234);
        CHECK_EQ(pack->stage(), 2);

        crow::deallocate_packet(pack);
        crow::disengage_packet_pool();
    }

    SUBCASE("switch_back_to_malloc")
    {
        // Start with pool
        crow::engage_packet_pool(pool_buffer, sizeof(pool_buffer), elem_size);
        CHECK(crow::is_pool_engaged());

        crow::packet *p1 = crow::allocate_packet_header_v1(64);
        CHECK(p1 != nullptr);
        crow::deallocate_packet(p1);

        // Switch back to malloc
        crow::disengage_packet_pool();
        CHECK_FALSE(crow::is_pool_engaged());

        // Allocate with malloc
        crow::packet *p2 = crow::allocate_packet_header_v1(64);
        CHECK(p2 != nullptr);
        crow::deallocate_packet(p2);
    }

    // Ensure we're back to malloc mode for other tests
    crow::disengage_packet_pool();
    crow::reset_for_test();
}
