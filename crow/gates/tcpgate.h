#ifndef CROW_GATES_TCPGATE_H
#define CROW_GATES_TCPGATE_H

#include <crow/gateway.h>
#include <igris/container/unbounded_array.h>
#include <map>
#include <memory>

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#define CROW_TCPGATE_NO 13

namespace crow
{
    /**
     * TCP gate for crow transport.
     *
     * Protocol framing: each crow packet is prefixed with 4-byte length
     * (network byte order, big-endian).
     *
     * Address format in crow packet stage area:
     *   [1 byte: gate_id=13][4 bytes: IPv4 addr][2 bytes: port (network order)]
     */
    class tcpgate : public gateway
    {
        // Connection state for each remote peer
        struct connection
        {
            int fd = -1;
            igris::unbounded_array<uint8_t> recv_buffer = {};
            size_t expected_size = 0; // 0 = waiting for header
        };

        int server_fd = -1;
        uint16_t listen_port = 0;
        std::map<uint64_t, connection> connections = {}; // key = (ip << 16) | port
        igris::unbounded_array<uint8_t> send_buffer = {};
        bool _debug = false;

        // Make key from IP and port
        static uint64_t make_key(uint32_t ip, uint16_t port)
        {
            return ((uint64_t)ip << 16) | port;
        }

        // Extract IP and port from stage pointer
        static void parse_stage(const uint8_t *stage, uint32_t &ip,
                                uint16_t &port)
        {
            ip = *(uint32_t *)(stage + 1);
            port = *(uint16_t *)(stage + 5);
        }

    public:
        tcpgate() {}
        tcpgate(uint16_t port) { open(port); }

        void send(crow::packet *) override;

        bool opened() { return server_fd > 0; }

        int open(uint16_t port = 0);
        void close();
        void finish() override { close(); }

        int bind(Tower &tower, int gate_no = CROW_TCPGATE_NO)
        {
            return gateway::bind(tower, gate_no);
        }

        void debug(bool val) { _debug = val; }

        // Asyncio handlers
        void accept_handler(int fd);
        void read_handler(int fd);

        // Find or create connection to remote peer
        int get_or_create_connection(uint32_t ip, uint16_t port);

        // Close specific connection
        void close_connection(uint64_t key);

        ~tcpgate() override { close(); }
    };

    std::shared_ptr<crow::tcpgate> create_tcpgate_safe(uint8_t id = CROW_TCPGATE_NO,
                                                       uint16_t port = 0);
}

#endif
