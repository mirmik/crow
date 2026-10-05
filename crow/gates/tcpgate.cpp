/** @file */

#include <crow/defs.h>
#include <crow/gates/tcpgate.h>
#include <crow/tower_cls.h>

#include <fcntl.h>
#include <unistd.h>

#ifdef __WIN32__
#include <winsock2.h>
#define SHUT_RDWR 2
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

#include <cstring>
#include <memory>
#include <nos/io/stdfile.h>
#include <nos/print.h>
#include <nos/util/osutil.h>

#ifdef CROW_USE_ASYNCIO
#include <crow/asyncio.h>
#endif

// Frame header size: 4 bytes for packet length
static constexpr size_t FRAME_HEADER_SIZE = 4;

std::shared_ptr<crow::tcpgate> crow::create_tcpgate_safe(uint8_t id,
                                                          uint16_t port)
{
    (void)id; // gate id is set when binding to tower

    auto g = std::make_shared<crow::tcpgate>();
    if (g->open(port) != 0)
    {
        return nullptr;
    }
    return g;
}

int crow::tcpgate::open(uint16_t port)
{
    listen_port = port;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("tcpgate socket");
        return -1;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (port != 0)
    {
        if (::bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("tcpgate bind");
            ::close(server_fd);
            server_fd = -1;
            return -1;
        }
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("tcpgate listen");
        ::close(server_fd);
        server_fd = -1;
        return -1;
    }

    nos::osutil::nonblock(server_fd, true);

#ifdef CROW_USE_ASYNCIO
    crow::asyncio.add_iotask(
        server_fd,
        SelectType::READ,
        igris::make_delegate(&tcpgate::accept_handler, this));
#endif

    if (_debug)
        nos::println_to(nos::cerr, "tcpgate: listening on port ", port);

    return 0;
}

void crow::tcpgate::accept_handler(int fd)
{
    (void)fd;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("tcpgate accept");
        return;
    }

    // Set non-blocking
    nos::osutil::nonblock(client_fd, true);

    // Disable Nagle's algorithm for low latency
    int flag = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    uint32_t ip = client_addr.sin_addr.s_addr;
    uint16_t port = client_addr.sin_port;
    uint64_t key = make_key(ip, port);

    if (_debug)
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str));
        nos::println_to(nos::cerr, "tcpgate: accepted connection from ", ip_str, ":", ntohs(port));
    }

    // Store connection
    connection conn;
    conn.fd = client_fd;
    conn.expected_size = 0;
    connections[key] = std::move(conn);

#ifdef CROW_USE_ASYNCIO
    crow::asyncio.add_iotask(
        client_fd,
        SelectType::READ,
        igris::make_delegate(&tcpgate::read_handler, this));
#endif
}

void crow::tcpgate::read_handler(int fd)
{
    if (_debug)
        nos::println_to(nos::cerr, "tcpgate: read_handler called for fd=", fd);

    // Find connection by fd
    uint64_t conn_key = 0;
    connection *conn = nullptr;

    for (auto &kv : connections)
    {
        if (kv.second.fd == fd)
        {
            conn_key = kv.first;
            conn = &kv.second;
            break;
        }
    }

    if (!conn)
    {
        if (_debug)
            nos::println_to(nos::cerr, "tcpgate: unknown fd, discarding");
        // Unknown fd, just read and discard
        char buf[1024];
        read(fd, buf, sizeof(buf));
        return;
    }

    // Read available data
    char buf[4096];
    ssize_t len = read(fd, buf, sizeof(buf));

    if (_debug)
        nos::println_to(nos::cerr, "tcpgate: read ", len, " bytes from fd=", fd);

    if (len <= 0)
    {
        if (len == 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
        {
            if (_debug)
                nos::println_to(nos::cerr, "tcpgate: connection closed, fd=", fd);
            close_connection(conn_key);
        }
        return;
    }

    // Append to receive buffer
    size_t old_size = conn->recv_buffer.size();
    conn->recv_buffer.resize(old_size + len);
    memcpy(conn->recv_buffer.data() + old_size, buf, len);

    // Process complete frames
    while (true)
    {
        size_t buf_size = conn->recv_buffer.size();

        // Need frame header first
        if (conn->expected_size == 0)
        {
            if (buf_size < FRAME_HEADER_SIZE)
                break;

            // Read 4-byte length (network byte order)
            uint32_t frame_len;
            memcpy(&frame_len, conn->recv_buffer.data(), 4);
            conn->expected_size = ntohl(frame_len);

            if (_debug)
            {
                nos::println_to(nos::cerr, "tcpgate: frame header raw bytes: ",
                    (int)(unsigned char)conn->recv_buffer[0], " ",
                    (int)(unsigned char)conn->recv_buffer[1], " ",
                    (int)(unsigned char)conn->recv_buffer[2], " ",
                    (int)(unsigned char)conn->recv_buffer[3]);
                nos::println_to(nos::cerr, "tcpgate: frame header, size=", conn->expected_size);
            }
        }

        // Check if we have complete frame
        size_t total_needed = FRAME_HEADER_SIZE + conn->expected_size;
        if (buf_size < total_needed)
            break;

        // Extract crow packet data (after frame header)
        uint8_t *frame_data = conn->recv_buffer.data() + FRAME_HEADER_SIZE;
        size_t frame_size = conn->expected_size;

        // Parse crow header to allocate packet
        crow::header_v1 header;
        if (frame_size < sizeof(header))
        {
            if (_debug)
                nos::println_to(nos::cerr, "tcpgate: frame too small for header");
            // Remove bad frame and continue
            conn->recv_buffer.resize(0);
            conn->expected_size = 0;
            break;
        }

        memcpy(&header, frame_data, sizeof(header));

        // Allocate packet
        crow::packet *pack =
            allocate_packet<crow::header_v1>(header.addrsize(), header.datasize());
        pack->parse_header(header);

        // Copy full packet data
        memcpy(pack->header_addr(), frame_data, frame_size);

        crow::packet_initialization(pack, this);

        // Add sender address to packet (revert stage)
        uint32_t sender_ip = (uint32_t)(conn_key >> 16);
        uint16_t sender_port = (uint16_t)(conn_key & 0xFFFF);

        nos::buffer vec[3] = {{(char *)&id, 1},
                              {(char *)&sender_ip, 4},
                              {(char *)&sender_port, 2}};

        pack->revert(vec, 3);

        if (_debug)
            nos::println_to(nos::cerr, "tcpgate: received packet, size=", frame_size);

        // Remove processed frame from buffer BEFORE calling tower
        // (tower callback may modify connections map, invalidating conn pointer)
        size_t remaining = buf_size - total_needed;
        if (remaining > 0)
        {
            memmove(conn->recv_buffer.data(),
                    conn->recv_buffer.data() + total_needed,
                    remaining);
        }
        conn->recv_buffer.resize(remaining);
        conn->expected_size = 0;

        // Pass to tower (this may trigger send() which modifies connections)
        _tower->nocontrol_travel(pack, true);

        // Re-find connection after tower callback (map may have been modified)
        auto it = connections.find(conn_key);
        if (it == connections.end())
        {
            // Connection was closed during processing
            return;
        }
        conn = &it->second;
    }
}

int crow::tcpgate::get_or_create_connection(uint32_t ip, uint16_t port)
{
    uint64_t key = make_key(ip, port);

    auto it = connections.find(key);
    if (it != connections.end() && it->second.fd >= 0)
    {
        return it->second.fd;
    }

    // Create new connection
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("tcpgate socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;

    if (_debug)
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str));
        nos::println_to(nos::cerr, "tcpgate: connecting to ", ip_str, ":", ntohs(port));
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("tcpgate connect");
        ::close(sock);
        return -1;
    }

    // Set non-blocking after connect
    nos::osutil::nonblock(sock, true);

    // Disable Nagle
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    // Store connection
    connection conn;
    conn.fd = sock;
    conn.expected_size = 0;
    connections[key] = std::move(conn);

#ifdef CROW_USE_ASYNCIO
    crow::asyncio.add_iotask(
        sock,
        SelectType::READ,
        igris::make_delegate(&tcpgate::read_handler, this));
#endif

    if (_debug)
        nos::println_to(nos::cerr, "tcpgate: connected, fd=", sock);

    return sock;
}

void crow::tcpgate::send(crow::packet *pack)
{
    uint32_t ip;
    uint16_t port;
    parse_stage((uint8_t *)pack->stageptr(), ip, port);

    if (_debug)
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ip_str, sizeof(ip_str));
        nos::println_to(nos::cerr, "tcpgate::send: to ", ip_str, ":", ntohs(port));
    }

    int sock = get_or_create_connection(ip, port);
    if (sock < 0)
    {
        if (_debug)
            nos::println_to(nos::cerr, "tcpgate::send: failed to get connection");
        _tower->return_to_tower(pack, CROW_WRONG_ADDRESS);
        return;
    }

    // Build frame: 4-byte length + crow packet
    header_v1 header = pack->extract_header_v1();
    uint32_t packet_size = header.flen;
    uint32_t frame_len = htonl(packet_size);

    // Resize send buffer
    if (send_buffer.size() < FRAME_HEADER_SIZE + packet_size)
        send_buffer.resize(FRAME_HEADER_SIZE + packet_size);

    // Write frame header
    memcpy(send_buffer.data(), &frame_len, 4);

    // Write crow packet
    memcpy(send_buffer.data() + FRAME_HEADER_SIZE, &header, sizeof(header));
    memcpy(send_buffer.data() + FRAME_HEADER_SIZE + sizeof(header),
           pack->addrptr(),
           pack->addrsize());
    memcpy(send_buffer.data() + FRAME_HEADER_SIZE + sizeof(header) + pack->addrsize(),
           pack->dataptr(),
           pack->datasize());

    // Send with retry for partial writes
    size_t total = FRAME_HEADER_SIZE + packet_size;
    size_t sent = 0;

    while (sent < total)
    {
        ssize_t n = ::send(sock, (char *)send_buffer.data() + sent, total - sent, 0);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Would block, try again
                usleep(1000);
                continue;
            }
            perror("tcpgate send");
            _tower->return_to_tower(pack, CROW_WRONG_ADDRESS);
            return;
        }
        sent += n;
    }

    if (_debug)
        nos::println_to(nos::cerr, "tcpgate: sent frame, size=", packet_size);

    _tower->return_to_tower(pack, CROW_SENDED);
}

void crow::tcpgate::close_connection(uint64_t key)
{
    auto it = connections.find(key);
    if (it != connections.end())
    {
        if (it->second.fd >= 0)
        {
#ifdef CROW_USE_ASYNCIO
            crow::asyncio.remove_iotask(it->second.fd);
#endif
            shutdown(it->second.fd, SHUT_RDWR);
            ::close(it->second.fd);
        }
        connections.erase(it);
    }
}

void crow::tcpgate::close()
{
    // Close all client connections
    for (auto &kv : connections)
    {
        if (kv.second.fd >= 0)
        {
#ifdef CROW_USE_ASYNCIO
            crow::asyncio.remove_iotask(kv.second.fd);
#endif
            shutdown(kv.second.fd, SHUT_RDWR);
            ::close(kv.second.fd);
        }
    }
    connections.clear();

    // Close server socket
    if (server_fd >= 0)
    {
#ifdef CROW_USE_ASYNCIO
        crow::asyncio.remove_iotask(server_fd);
#endif
        shutdown(server_fd, SHUT_RDWR);
        ::close(server_fd);
        server_fd = -1;
    }
}
