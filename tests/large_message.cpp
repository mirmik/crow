/**
 * @file large_message.cpp
 * @brief Comprehensive test for large message transmission through service_node
 *
 * Tests chunked reply mechanism with various message sizes, chunk sizes,
 * and edge cases. Tests focus on chunked message building (service_node side)
 * and reassembly (subscriber_node/requestor_node side).
 */

#include <crow/nodes/requestor_node.h>
#include <crow/nodes/service_node.h>
#include <crow/nodes/subscriber_node.h>
#include <crow/address.h>
#include <crow/tower.h>
#include <doctest/doctest.h>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include "allocator_test_helper.h"

// Helper to generate test data of specified size
static std::string generate_test_data(size_t size)
{
    std::string data;
    data.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        // Use printable ASCII characters for easier debugging
        data.push_back(static_cast<char>('A' + (i % 26)));
    }
    return data;
}

// Helper to verify received data matches expected
static bool verify_data(const std::string &received, const std::string &expected)
{
    if (received.size() != expected.size())
        return false;
    return std::memcmp(received.data(), expected.data(), received.size()) == 0;
}

// Helper to build a chunk from data
static std::vector<char> build_chunk(const std::string& data, size_t offset,
                                      size_t payload_size, uint16_t chunk_id, bool has_more)
{
    size_t actual_payload = std::min(payload_size, data.size() - offset);

    std::vector<char> chunk;
    chunk.push_back(static_cast<char>(crow::CHUNKED_REPLY_MARKER));
    chunk.push_back(static_cast<char>(chunk_id & 0xFF));
    chunk.push_back(static_cast<char>((chunk_id >> 8) & 0xFF));
    chunk.push_back(has_more ? crow::CHUNK_FLAG_HAS_MORE : 0);
    chunk.insert(chunk.end(), data.begin() + offset,
                data.begin() + offset + actual_payload);

    return chunk;
}

// Helper to build all chunks for a message
static std::vector<std::vector<char>> build_all_chunks(const std::string& data,
                                                        size_t payload_per_chunk)
{
    std::vector<std::vector<char>> chunks;
    size_t offset = 0;
    uint16_t chunk_id = 0;

    while (offset < data.size())
    {
        size_t chunk_payload = std::min(payload_per_chunk, data.size() - offset);
        bool has_more = (offset + chunk_payload < data.size());

        chunks.push_back(build_chunk(data, offset, payload_per_chunk, chunk_id, has_more));

        offset += chunk_payload;
        chunk_id++;
    }

    return chunks;
}

// ============================================================================
// Unit tests for chunked message building (service_node side)
// ============================================================================

TEST_CASE("large_message_chunk_building")
{
    SUBCASE("chunk_header_constants")
    {
        CHECK_EQ(crow::CHUNKED_REPLY_MARKER, 0x01);
        CHECK_EQ(crow::CHUNK_FLAG_HAS_MORE, 0x01);
    }

    SUBCASE("service_node_chunk_size_configuration")
    {
        crow::service_node service;

        // Default is no chunking
        CHECK_EQ(service.chunk_size(), 0);

        // Set various chunk sizes
        service.set_chunk_size(100);
        CHECK_EQ(service.chunk_size(), 100);

        service.set_chunk_size(256);
        CHECK_EQ(service.chunk_size(), 256);

        service.set_chunk_size(512);
        CHECK_EQ(service.chunk_size(), 512);

        // Disable chunking
        service.set_chunk_size(0);
        CHECK_EQ(service.chunk_size(), 0);
    }

    SUBCASE("chunk_header_format_marker")
    {
        // Verify marker byte value
        std::vector<char> chunk = {0x01, 0x00, 0x00, 0x00, 'X'};
        CHECK_EQ(static_cast<uint8_t>(chunk[0]), crow::CHUNKED_REPLY_MARKER);
    }

    SUBCASE("chunk_header_format_id_little_endian")
    {
        // Chunk ID is little-endian 16-bit
        uint16_t chunk_id = 0x1234;
        std::vector<char> chunk;
        chunk.push_back(static_cast<char>(crow::CHUNKED_REPLY_MARKER));
        chunk.push_back(static_cast<char>(chunk_id & 0xFF));         // 0x34
        chunk.push_back(static_cast<char>((chunk_id >> 8) & 0xFF));  // 0x12
        chunk.push_back(0);

        CHECK_EQ(static_cast<uint8_t>(chunk[1]), 0x34);
        CHECK_EQ(static_cast<uint8_t>(chunk[2]), 0x12);

        // Parse back
        uint16_t parsed_id = static_cast<uint8_t>(chunk[1]) |
                           (static_cast<uint8_t>(chunk[2]) << 8);
        CHECK_EQ(parsed_id, chunk_id);
    }

    SUBCASE("chunk_header_format_flags")
    {
        // has_more flag
        std::vector<char> chunk_more = {0x01, 0x00, 0x00, 0x01};
        std::vector<char> chunk_last = {0x01, 0x00, 0x00, 0x00};

        CHECK_EQ(static_cast<uint8_t>(chunk_more[3]) & crow::CHUNK_FLAG_HAS_MORE,
                 crow::CHUNK_FLAG_HAS_MORE);
        CHECK_EQ(static_cast<uint8_t>(chunk_last[3]) & crow::CHUNK_FLAG_HAS_MORE, 0);
    }
}

// ============================================================================
// Unit tests for chunked message reassembly (subscriber_node side)
// ============================================================================

TEST_CASE("large_message_reassembly_subscriber")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("reassemble_100_byte_message_in_20_byte_chunks")
    {
        std::string original = generate_test_data(100);
        auto chunks = build_all_chunks(original, 20);

        CHECK_EQ(chunks.size(), 5);

        // Send all chunks in order
        for (size_t i = 0; i < chunks.size() - 1; ++i)
        {
            node.handle_incoming_message(
                nos::buffer(chunks[i].data(), chunks[i].size()));
            CHECK_FALSE(callback_called);
        }

        // Last chunk should trigger callback
        node.handle_incoming_message(
            nos::buffer(chunks.back().data(), chunks.back().size()));
        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("reassemble_1000_byte_message")
    {
        std::string original = generate_test_data(1000);
        auto chunks = build_all_chunks(original, 100);

        CHECK_EQ(chunks.size(), 10);

        // Send all chunks
        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("reassemble_out_of_order_10_chunks")
    {
        std::string original = generate_test_data(200);
        auto chunks = build_all_chunks(original, 20);

        CHECK_EQ(chunks.size(), 10);

        // Send in reverse order (except last chunk)
        // Order: 8, 6, 4, 2, 0, 7, 5, 3, 1, 9(last)
        std::vector<size_t> order = {8, 6, 4, 2, 0, 7, 5, 3, 1, 9};

        for (size_t i = 0; i < order.size() - 1; ++i)
        {
            auto &chunk = chunks[order[i]];
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
            CHECK_FALSE(callback_called);
        }

        // Last chunk completes reassembly
        auto &last_chunk = chunks[order.back()];
        node.handle_incoming_message(nos::buffer(last_chunk.data(), last_chunk.size()));
        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("reassemble_last_chunk_arrives_first")
    {
        std::string original = generate_test_data(60);
        auto chunks = build_all_chunks(original, 20);

        CHECK_EQ(chunks.size(), 3);

        // Send last chunk first
        node.handle_incoming_message(
            nos::buffer(chunks[2].data(), chunks[2].size()));
        CHECK_FALSE(callback_called);

        // Then middle chunk
        node.handle_incoming_message(
            nos::buffer(chunks[1].data(), chunks[1].size()));
        CHECK_FALSE(callback_called);

        // Finally first chunk - should complete
        node.handle_incoming_message(
            nos::buffer(chunks[0].data(), chunks[0].size()));
        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("reassemble_2000_byte_message_small_chunks")
    {
        // Stress test: 2000 bytes in 25-byte chunks = 80 chunks
        std::string original = generate_test_data(2000);
        auto chunks = build_all_chunks(original, 25);

        CHECK_EQ(chunks.size(), 80);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("reassemble_exact_boundary_96_bytes_in_32_chunks")
    {
        // 96 bytes / 32 = 3 chunks exactly
        std::string original = generate_test_data(96);
        auto chunks = build_all_chunks(original, 32);

        CHECK_EQ(chunks.size(), 3);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }
}

// ============================================================================
// Unit tests for chunked message reassembly (requestor_node side)
// ============================================================================

// Helper class for requestor_node tests since igris::delegate doesn't support lambdas with capture
struct RequestorTestHelper
{
    std::string received_data;
    bool callback_called = false;

    void callback(nos::buffer data)
    {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    }

    void reset()
    {
        received_data.clear();
        callback_called = false;
    }
};

TEST_CASE("large_message_reassembly_requestor")
{
    RequestorTestHelper helper;

    crow::requestor_node node;
    node.set_async_handle(
        igris::delegate<void, nos::buffer>(&RequestorTestHelper::callback, &helper));

    SUBCASE("single_legacy_message")
    {
        std::string msg = "Response";
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, "Response");
    }

    SUBCASE("two_chunks_in_order")
    {
        std::string original = "ABCDEF";
        auto chunks = build_all_chunks(original, 3);

        CHECK_EQ(chunks.size(), 2);

        node.handle_incoming_message(nos::buffer(chunks[0].data(), chunks[0].size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunks[1].data(), chunks[1].size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, original);
    }

    SUBCASE("two_chunks_out_of_order")
    {
        std::string original = "ABCDEF";
        auto chunks = build_all_chunks(original, 3);

        node.handle_incoming_message(nos::buffer(chunks[1].data(), chunks[1].size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunks[0].data(), chunks[0].size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, original);
    }

    SUBCASE("five_chunks_random_order")
    {
        std::string original = "01234";
        auto chunks = build_all_chunks(original, 1);

        CHECK_EQ(chunks.size(), 5);

        // Send in random order: 3, 1, 4, 0, 2
        std::vector<size_t> order = {3, 1, 4, 0, 2};

        for (size_t i = 0; i < order.size() - 1; ++i)
        {
            node.handle_incoming_message(
                nos::buffer(chunks[order[i]].data(), chunks[order[i]].size()));
            CHECK_FALSE(helper.callback_called);
        }

        // Last one completes
        size_t last_idx = order.back();
        node.handle_incoming_message(
            nos::buffer(chunks[last_idx].data(), chunks[last_idx].size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, original);
    }

    SUBCASE("large_message_500_bytes")
    {
        std::string original = generate_test_data(500);
        auto chunks = build_all_chunks(original, 50);

        CHECK_EQ(chunks.size(), 10);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(helper.callback_called);
        CHECK(verify_data(helper.received_data, original));
    }
}

// ============================================================================
// Binary data tests
// ============================================================================

TEST_CASE("large_message_binary_data")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("binary_data_with_null_bytes")
    {
        // Create data with embedded null bytes
        std::string original(100, '\0');
        for (size_t i = 0; i < original.size(); ++i)
        {
            original[i] = static_cast<char>(i % 256);
        }

        auto chunks = build_all_chunks(original, 20);
        CHECK_EQ(chunks.size(), 5);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK_EQ(received_data.size(), original.size());
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("all_0xff_bytes")
    {
        std::string original(50, '\xFF');
        auto chunks = build_all_chunks(original, 25);

        CHECK_EQ(chunks.size(), 2);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("marker_byte_in_payload")
    {
        // Data starting with CHUNKED_REPLY_MARKER but longer than header
        std::string original;
        original.push_back(static_cast<char>(crow::CHUNKED_REPLY_MARKER));
        original.append(generate_test_data(49));  // Total 50 bytes

        auto chunks = build_all_chunks(original, 25);
        CHECK_EQ(chunks.size(), 2);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }
}

// ============================================================================
// Edge cases and error handling
// ============================================================================

TEST_CASE("large_message_edge_cases")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("single_byte_payload_per_chunk")
    {
        std::string original = "HELLO";
        auto chunks = build_all_chunks(original, 1);

        CHECK_EQ(chunks.size(), 5);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK_EQ(received_data, original);
    }

    SUBCASE("empty_chunk_payload")
    {
        // Single empty chunk
        std::vector<char> chunk = {0x01, 0x00, 0x00, 0x00};  // marker, id=0, flags=0 (last)
        node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));

        CHECK(callback_called);
        CHECK(received_data.empty());
    }

    SUBCASE("multiple_empty_chunks")
    {
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01};  // empty, has_more
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x00};  // empty, last

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(callback_called);

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK(callback_called);
        CHECK(received_data.empty());
    }

    SUBCASE("legacy_message_resets_chunk_state")
    {
        // Start receiving chunks
        std::string original = generate_test_data(100);
        auto chunks = build_all_chunks(original, 25);

        // Send first two chunks
        node.handle_incoming_message(nos::buffer(chunks[0].data(), chunks[0].size()));
        node.handle_incoming_message(nos::buffer(chunks[1].data(), chunks[1].size()));
        CHECK_FALSE(callback_called);

        // Then receive a legacy message - should reset and process
        std::string legacy = "Legacy";
        node.handle_incoming_message(nos::buffer(legacy.data(), legacy.size()));
        CHECK(callback_called);
        CHECK_EQ(received_data, "Legacy");
    }

    SUBCASE("message_starting_with_0x01_but_too_short")
    {
        // Message starting with marker but less than 4 bytes = legacy
        std::vector<char> msg = {0x01, 0x02, 0x03};
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data.size(), 3);
        CHECK_EQ(static_cast<uint8_t>(received_data[0]), 0x01);
    }

    SUBCASE("single_byte_0x01_message")
    {
        std::vector<char> msg = {0x01};
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data.size(), 1);
    }

    SUBCASE("max_chunk_id_limit")
    {
        // Chunk with ID >= MAX_CHUNKS should be rejected
        uint16_t high_id = crow::MAX_CHUNKS;  // 256
        std::vector<char> chunk = {
            0x01,
            static_cast<char>(high_id & 0xFF),
            static_cast<char>((high_id >> 8) & 0xFF),
            0x00,  // last chunk
            'X'
        };

        node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));

        // Should be rejected - no callback
        CHECK_FALSE(callback_called);
    }
}

// ============================================================================
// Stress tests
// ============================================================================

TEST_CASE("large_message_stress")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("max_allowed_chunks")
    {
        // Test with MAX_CHUNKS - 1 (255) chunks
        // 255 chunks * 10 bytes = 2550 bytes
        std::string original = generate_test_data(2550);
        auto chunks = build_all_chunks(original, 10);

        CHECK_EQ(chunks.size(), 255);

        for (auto &chunk : chunks)
        {
            node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
        }

        CHECK(callback_called);
        CHECK(verify_data(received_data, original));
    }

    SUBCASE("repeated_transmissions")
    {
        // Test multiple complete transmissions in a row
        for (int iter = 0; iter < 10; ++iter)
        {
            callback_called = false;
            received_data.clear();

            std::string original = generate_test_data(100 + iter * 10);
            auto chunks = build_all_chunks(original, 20);

            for (auto &chunk : chunks)
            {
                node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
            }

            CHECK(callback_called);
            CHECK(verify_data(received_data, original));
        }
    }

    SUBCASE("interleaved_with_legacy_messages")
    {
        // Test that legacy messages work correctly between chunked transmissions
        for (int iter = 0; iter < 5; ++iter)
        {
            // First send chunked message
            callback_called = false;
            received_data.clear();

            std::string original = generate_test_data(50);
            auto chunks = build_all_chunks(original, 10);

            for (auto &chunk : chunks)
            {
                node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));
            }

            CHECK(callback_called);
            CHECK(verify_data(received_data, original));

            // Then send legacy message
            callback_called = false;
            received_data.clear();

            std::string legacy = "Legacy" + std::to_string(iter);
            node.handle_incoming_message(nos::buffer(legacy.data(), legacy.size()));

            CHECK(callback_called);
            CHECK_EQ(received_data, legacy);
        }
    }
}
