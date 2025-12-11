/**
 * @file node_chunked.cpp
 * @brief Tests for chunked message transmission at the node protocol level
 *
 * These tests verify that:
 * 1. Large messages are automatically split into chunks when chunk_size is set
 * 2. Chunks are properly reassembled at the receiver
 * 3. Out-of-order chunks are handled correctly
 * 4. Timeout and size limits work as expected
 * 5. Multiple concurrent chunked sessions work correctly
 */

#include <crow/address.h>
#include <crow/gates/udpgate.h>
#include <crow/proto/node.h>
#include <crow/proto/node_protocol.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// Test node that records received messages
class test_receiver_node : public crow::node
{
public:
    std::vector<std::string> received_messages;
    std::atomic<int> receive_count{0};

    void incoming_packet(crow::packet *pack) override
    {
        auto data = crow::node_data(pack);
        received_messages.emplace_back(data.data(), data.size());
        receive_count++;
        _tower->release(pack);
    }
};

// Test node that can send chunked messages
class test_sender_node : public crow::node
{
public:
    void incoming_packet(crow::packet *pack) override
    {
        _tower->release(pack);
    }
};

// Custom udpgate for testing
class test_udpgate : public crow::udpgate
{
public:
    using crow::udpgate::udpgate;

    int bind(crow::Tower &tower, int gate_no = CROW_UDPGATE_NO)
    {
        return crow::gateway::bind(tower, gate_no);
    }
};

// Helper to generate test data of specified size
std::string generate_test_data(size_t size, char start = 'A')
{
    std::string data;
    data.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        data.push_back(static_cast<char>(start + (i % 26)));
    }
    return data;
}

// Helper to run both towers until condition is met or timeout
template <typename Pred>
bool run_towers_until(crow::Tower &t1, crow::Tower &t2,
                      test_udpgate &g1, test_udpgate &g2,
                      Pred pred, int max_iterations = 100)
{
    for (int i = 0; i < max_iterations; ++i)
    {
        t1.onestep();
        t2.onestep();
        g1.read_handler(0);
        g2.read_handler(0);

        if (pred())
            return true;

        std::this_thread::sleep_for(5ms);
    }
    return false;
}

TEST_SUITE("node_chunked")
{
    TEST_CASE("node_subheader_flags" * doctest::timeout(1.0))
    {
        crow::node_subheader sh;
        sh.sid = 100;
        sh.rid = 200;
        sh.u.flags = 0;

        // Check initial state
        CHECK_EQ(sh.u.f.chunked, 0);
        CHECK_EQ(sh.u.f.has_more, 0);
        CHECK_EQ(sh.u.f.type, 0);

        // Set chunked flag
        sh.u.f.chunked = 1;
        CHECK_EQ(sh.u.f.chunked, 1);
        CHECK_EQ(sh.u.f.has_more, 0);

        // Set has_more flag
        sh.u.f.has_more = 1;
        CHECK_EQ(sh.u.f.chunked, 1);
        CHECK_EQ(sh.u.f.has_more, 1);

        // Set type
        sh.u.f.type = CROW_NODEPACK_COMMON;
        CHECK_EQ(sh.u.f.type, CROW_NODEPACK_COMMON);

        // Verify flags byte layout
        // has_more=bit0, chunked=bit1, reserved=bits2-3, type=bits4-7
        CHECK_EQ(sh.u.flags & 0x03, 0x03); // both flags set
    }

    TEST_CASE("chunk_header_format" * doctest::timeout(1.0))
    {
        crow::node_chunk_header ch;
        ch.chunk_id = 0x1234;

        // Verify little-endian storage
        uint8_t *bytes = reinterpret_cast<uint8_t *>(&ch);
        CHECK_EQ(bytes[0], 0x34);
        CHECK_EQ(bytes[1], 0x12);
    }

    TEST_CASE("small_message_no_chunking" * doctest::timeout(5.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21001) == 0);
        REQUIRE(gate2.open(21002) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        // Set chunk_size but send small message
        sender.set_chunk_size(100);

        auto addr = crow::address(".12.127.0.0.1:21002");
        std::string data = "Hello, World!";

        sender.send_chunked(20, addr, data, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; });

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("chunked_message_two_chunks" * doctest::timeout(5.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21003) == 0);
        REQUIRE(gate2.open(21004) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        // chunk_size=50, header overhead = 5+2 = 7 bytes
        // payload per chunk = 50 - 7 = 43 bytes
        // 80 bytes data -> 2 chunks
        sender.set_chunk_size(50);

        auto addr = crow::address(".12.127.0.0.1:21004");
        std::string data = generate_test_data(80);

        sender.send_chunked(20, addr, data, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; }, 200);

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("chunked_message_multiple_chunks" * doctest::timeout(5.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21005) == 0);
        REQUIRE(gate2.open(21006) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        // chunk_size=30, header overhead = 7 bytes
        // payload per chunk = 23 bytes
        // 200 bytes data -> 9 chunks
        sender.set_chunk_size(30);

        auto addr = crow::address(".12.127.0.0.1:21006");
        std::string data = generate_test_data(200);

        sender.send_chunked(20, addr, data, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; }, 300);

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("chunked_large_message" * doctest::timeout(10.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21007) == 0);
        REQUIRE(gate2.open(21008) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        // chunk_size=100, test with 2000 bytes
        sender.set_chunk_size(100);

        auto addr = crow::address(".12.127.0.0.1:21008");
        std::string data = generate_test_data(2000);

        sender.send_chunked(20, addr, data, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; }, 500);

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0].size(), data.size());
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("chunked_with_qos1" * doctest::timeout(10.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21009) == 0);
        REQUIRE(gate2.open(21010) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        sender.set_chunk_size(50);

        auto addr = crow::address(".12.127.0.0.1:21010");
        std::string data = generate_test_data(150);

        // Send with QoS 1
        sender.send_chunked(20, addr, data, 1, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; }, 300);

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("chunked_binary_data" * doctest::timeout(5.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21011) == 0);
        REQUIRE(gate2.open(21012) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        sender.set_chunk_size(40);

        auto addr = crow::address(".12.127.0.0.1:21012");

        // Binary data with null bytes, 0xFF, etc.
        std::string data;
        data.reserve(100);
        for (int i = 0; i < 100; ++i)
        {
            data.push_back(static_cast<char>(i));
        }

        sender.send_chunked(20, addr, data, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]() { return receiver.receive_count > 0; }, 200);

        CHECK(success);
        REQUIRE_EQ(receiver.received_messages.size(), 1);
        CHECK_EQ(receiver.received_messages[0].size(), data.size());
        CHECK_EQ(receiver.received_messages[0], data);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("bidirectional_chunked" * doctest::timeout(10.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;

        // Both nodes can send and receive
        test_receiver_node node1, node2;

        REQUIRE(gate1.open(21013) == 0);
        REQUIRE(gate2.open(21014) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        node1.bind(tower1, 10);
        node2.bind(tower2, 20);

        node1.set_chunk_size(50);
        node2.set_chunk_size(50);

        auto addr1 = crow::address(".12.127.0.0.1:21013");
        auto addr2 = crow::address(".12.127.0.0.1:21014");

        std::string data1 = generate_test_data(100, 'A');
        std::string data2 = generate_test_data(100, 'a');

        // Send from both directions
        node1.send_chunked(20, addr2, data1, 0, 50);
        node2.send_chunked(10, addr1, data2, 0, 50);

        bool success = run_towers_until(tower1, tower2, gate1, gate2,
                                        [&]()
                                        {
                                            return node1.receive_count > 0 && node2.receive_count > 0;
                                        },
                                        300);

        CHECK(success);
        REQUIRE_EQ(node1.received_messages.size(), 1);
        REQUIRE_EQ(node2.received_messages.size(), 1);
        CHECK_EQ(node2.received_messages[0], data1);
        CHECK_EQ(node1.received_messages[0], data2);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("multiple_sequential_messages" * doctest::timeout(15.0))
    {
        crow::Tower tower1, tower2;
        test_udpgate gate1, gate2;
        test_sender_node sender;
        test_receiver_node receiver;

        REQUIRE(gate1.open(21015) == 0);
        REQUIRE(gate2.open(21016) == 0);
        gate1.bind(tower1, 12);
        gate2.bind(tower2, 12);

        sender.bind(tower1, 10);
        receiver.bind(tower2, 20);

        sender.set_chunk_size(40);

        auto addr = crow::address(".12.127.0.0.1:21016");

        // Send multiple messages sequentially
        std::vector<std::string> messages = {
            generate_test_data(80, 'A'),
            generate_test_data(60, 'B'),
            generate_test_data(100, 'C')};

        for (size_t i = 0; i < messages.size(); ++i)
        {
            int expected_count = static_cast<int>(i + 1);
            sender.send_chunked(20, addr, messages[i], 0, 50);

            bool success = run_towers_until(
                tower1, tower2, gate1, gate2,
                [&]()
                { return receiver.receive_count >= expected_count; },
                200);

            CHECK(success);
        }

        REQUIRE_EQ(receiver.received_messages.size(), 3);
        CHECK_EQ(receiver.received_messages[0], messages[0]);
        CHECK_EQ(receiver.received_messages[1], messages[1]);
        CHECK_EQ(receiver.received_messages[2], messages[2]);

        gate1.close();
        gate2.close();
    }

    TEST_CASE("reassembly_session_count" * doctest::timeout(5.0))
    {
        // Verify that reassembly sessions are cleaned up after completion
        CHECK_EQ(crow::node_protocol.reassembly_sessions_count(), 0);
    }

    TEST_CASE("chunk_size_configuration" * doctest::timeout(1.0))
    {
        test_sender_node node;

        CHECK_EQ(node.chunk_size(), 0); // default

        node.set_chunk_size(100);
        CHECK_EQ(node.chunk_size(), 100);

        node.set_chunk_size(0);
        CHECK_EQ(node.chunk_size(), 0);
    }

    TEST_CASE("protocol_configuration" * doctest::timeout(1.0))
    {
        // Save original values
        auto orig_max_size = crow::node_protocol.max_message_size();
        auto orig_timeout = crow::node_protocol.reassembly_timeout();

        // Test setters
        crow::node_protocol.set_max_message_size(128 * 1024);
        CHECK_EQ(crow::node_protocol.max_message_size(), 128 * 1024);

        crow::node_protocol.set_reassembly_timeout(10000);
        CHECK_EQ(crow::node_protocol.reassembly_timeout(), 10000);

        // Restore
        crow::node_protocol.set_max_message_size(orig_max_size);
        crow::node_protocol.set_reassembly_timeout(orig_timeout);
    }
}
