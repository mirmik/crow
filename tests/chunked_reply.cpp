#include <crow/nodes/requestor_node.h>
#include <crow/nodes/service_node.h>
#include <crow/nodes/subscriber_node.h>
#include <doctest/doctest.h>
#include <cstring>
#include <string>
#include <vector>

// Test chunked reply parsing and reassembly without network

TEST_CASE("chunked_reply_constants")
{
    CHECK_EQ(crow::CHUNKED_REPLY_MARKER, 0x01);
    CHECK_EQ(crow::CHUNK_FLAG_HAS_MORE, 0x01);
}

TEST_CASE("chunked_reply_header_format")
{
    // Test that we can build and parse chunk headers correctly
    // Header format: [marker:1][chunk_id_lo:1][chunk_id_hi:1][flags:1]

    SUBCASE("build_chunk_header")
    {
        std::vector<char> chunk(8);
        uint16_t chunk_id = 0x1234;
        bool has_more = true;

        chunk[0] = static_cast<char>(crow::CHUNKED_REPLY_MARKER);
        chunk[1] = static_cast<char>(chunk_id & 0xFF);
        chunk[2] = static_cast<char>((chunk_id >> 8) & 0xFF);
        chunk[3] = has_more ? crow::CHUNK_FLAG_HAS_MORE : 0;

        CHECK_EQ(static_cast<uint8_t>(chunk[0]), crow::CHUNKED_REPLY_MARKER);
        CHECK_EQ(static_cast<uint8_t>(chunk[1]), 0x34);
        CHECK_EQ(static_cast<uint8_t>(chunk[2]), 0x12);
        CHECK_EQ(static_cast<uint8_t>(chunk[3]), crow::CHUNK_FLAG_HAS_MORE);
    }

    SUBCASE("parse_chunk_header")
    {
        char data[] = {0x01, 0x34, 0x12, 0x01, 'H', 'i'};

        uint8_t marker = static_cast<uint8_t>(data[0]);
        uint16_t chunk_id = static_cast<uint8_t>(data[1]) |
                           (static_cast<uint8_t>(data[2]) << 8);
        uint8_t flags = static_cast<uint8_t>(data[3]);
        bool has_more = (flags & crow::CHUNK_FLAG_HAS_MORE) != 0;

        CHECK_EQ(marker, crow::CHUNKED_REPLY_MARKER);
        CHECK_EQ(chunk_id, 0x1234);
        CHECK(has_more);
    }

    SUBCASE("parse_last_chunk")
    {
        char data[] = {0x01, 0x05, 0x00, 0x00, 'E', 'n', 'd'};

        uint8_t marker = static_cast<uint8_t>(data[0]);
        uint16_t chunk_id = static_cast<uint8_t>(data[1]) |
                           (static_cast<uint8_t>(data[2]) << 8);
        uint8_t flags = static_cast<uint8_t>(data[3]);
        bool has_more = (flags & crow::CHUNK_FLAG_HAS_MORE) != 0;

        CHECK_EQ(marker, crow::CHUNKED_REPLY_MARKER);
        CHECK_EQ(chunk_id, 5);
        CHECK_FALSE(has_more);
    }
}

TEST_CASE("subscriber_node_chunk_reassembly")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("single_legacy_message")
    {
        // Non-chunked message (doesn't start with CHUNKED_REPLY_MARKER)
        std::string msg = "Hello World";
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data, "Hello World");
    }

    SUBCASE("single_chunk_message")
    {
        // Single chunk (chunk_id=0, has_more=false)
        std::vector<char> chunk = {0x01, 0x00, 0x00, 0x00, 'T', 'e', 's', 't'};
        node.handle_incoming_message(nos::buffer(chunk.data(), chunk.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data, "Test");
    }

    SUBCASE("two_chunks_in_order")
    {
        // Chunk 0: has_more=true
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'H', 'e', 'l', 'l', 'o'};
        // Chunk 1: has_more=false (last)
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x00, ' ', 'W', 'o', 'r', 'l', 'd'};

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(callback_called);  // Not complete yet

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK(callback_called);
        CHECK_EQ(received_data, "Hello World");
    }

    SUBCASE("two_chunks_out_of_order")
    {
        // Chunk 1 arrives first: has_more=false (last)
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x00, ' ', 'W', 'o', 'r', 'l', 'd'};
        // Chunk 0 arrives second: has_more=true
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'H', 'e', 'l', 'l', 'o'};

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK_FALSE(callback_called);  // Last chunk received but chunk 0 missing

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK(callback_called);
        CHECK_EQ(received_data, "Hello World");
    }

    SUBCASE("three_chunks")
    {
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'A', 'B', 'C'};
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x01, 'D', 'E', 'F'};
        std::vector<char> chunk2 = {0x01, 0x02, 0x00, 0x00, 'G', 'H', 'I'};

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(callback_called);

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK_FALSE(callback_called);

        node.handle_incoming_message(nos::buffer(chunk2.data(), chunk2.size()));
        CHECK(callback_called);
        CHECK_EQ(received_data, "ABCDEFGHI");
    }

    SUBCASE("legacy_message_after_incomplete_chunks")
    {
        // Start receiving chunks
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'X', 'Y', 'Z'};
        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(callback_called);

        // Then receive a legacy message - should reset and process
        std::string legacy = "Legacy";
        node.handle_incoming_message(nos::buffer(legacy.data(), legacy.size()));
        CHECK(callback_called);
        CHECK_EQ(received_data, "Legacy");
    }
}

TEST_CASE("service_node_chunk_size")
{
    crow::service_node service;

    SUBCASE("default_chunk_size_is_zero")
    {
        CHECK_EQ(service.chunk_size(), 0);
    }

    SUBCASE("set_chunk_size")
    {
        service.set_chunk_size(256);
        CHECK_EQ(service.chunk_size(), 256);
    }

    SUBCASE("set_chunk_size_to_zero_disables")
    {
        service.set_chunk_size(256);
        service.set_chunk_size(0);
        CHECK_EQ(service.chunk_size(), 0);
    }
}

TEST_CASE("chunk_id_wraparound")
{
    // Test that large chunk_ids work correctly (up to 65535)
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    // Use high chunk_id (simulating many chunks)
    uint16_t high_id = 65534;
    std::vector<char> chunk_high = {
        0x01,
        static_cast<char>(high_id & 0xFF),
        static_cast<char>((high_id >> 8) & 0xFF),
        0x00,  // last chunk
        'X'
    };

    // This will set expected_chunks = 65535, but only chunk 65534 is present
    // So reassembly should fail (we're missing chunks 0-65533)
    node.handle_incoming_message(nos::buffer(chunk_high.data(), chunk_high.size()));

    // Should not call callback because chunks 0-65533 are missing
    CHECK_FALSE(callback_called);
}

TEST_CASE("empty_payload_chunks")
{
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("single_empty_chunk")
    {
        // Chunk with just header, no payload
        std::vector<char> chunk = {0x01, 0x00, 0x00, 0x00};
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
}

TEST_CASE("message_starting_with_0x01_but_not_chunked")
{
    // Edge case: legacy message that happens to start with 0x01
    // but is less than 4 bytes (minimum chunk header size)
    std::string received_data;
    bool callback_called = false;

    crow::subscriber_node node([&](nos::buffer data) {
        received_data = std::string(data.data(), data.size());
        callback_called = true;
    });

    SUBCASE("three_byte_message_starting_with_0x01")
    {
        std::vector<char> msg = {0x01, 0x02, 0x03};
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data.size(), 3);
        CHECK_EQ(static_cast<uint8_t>(received_data[0]), 0x01);
    }

    SUBCASE("one_byte_message_0x01")
    {
        std::vector<char> msg = {0x01};
        node.handle_incoming_message(nos::buffer(msg.data(), msg.size()));

        CHECK(callback_called);
        CHECK_EQ(received_data.size(), 1);
    }
}

// ============================================================================
// requestor_node tests
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

TEST_CASE("requestor_node_chunk_reassembly")
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
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'A', 'B', 'C'};
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x00, 'D', 'E', 'F'};

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, "ABCDEF");
    }

    SUBCASE("two_chunks_out_of_order")
    {
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x00, 'D', 'E', 'F'};
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, 'A', 'B', 'C'};

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, "ABCDEF");
    }

    SUBCASE("five_chunks_random_order")
    {
        // Create 5 chunks with different payloads
        std::vector<char> chunk0 = {0x01, 0x00, 0x00, 0x01, '0'};
        std::vector<char> chunk1 = {0x01, 0x01, 0x00, 0x01, '1'};
        std::vector<char> chunk2 = {0x01, 0x02, 0x00, 0x01, '2'};
        std::vector<char> chunk3 = {0x01, 0x03, 0x00, 0x01, '3'};
        std::vector<char> chunk4 = {0x01, 0x04, 0x00, 0x00, '4'};  // last

        // Send in random order: 3, 1, 4, 0, 2
        node.handle_incoming_message(nos::buffer(chunk3.data(), chunk3.size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunk1.data(), chunk1.size()));
        CHECK_FALSE(helper.callback_called);

        node.handle_incoming_message(nos::buffer(chunk4.data(), chunk4.size()));
        CHECK_FALSE(helper.callback_called);  // Last chunk received but missing 0, 2

        node.handle_incoming_message(nos::buffer(chunk0.data(), chunk0.size()));
        CHECK_FALSE(helper.callback_called);  // Still missing 2

        node.handle_incoming_message(nos::buffer(chunk2.data(), chunk2.size()));
        CHECK(helper.callback_called);
        CHECK_EQ(helper.received_data, "01234");
    }
}
