/**
 * @file large_reply_service.cpp
 * @brief Test service that replies with large messages (chunked)
 *
 * Usage: large_reply_service [options]
 *   --udp PORT       UDP port (default: 0 = system assigned)
 *   --chunk SIZE     Chunk size for replies (default: 64)
 *   --crowker ADDR   Crowker address (default: .12.127.0.0.1:10009)
 *   --theme NAME     Service theme (default: "test_service")
 *   --debug          Enable debug output
 */

#include <crow/address.h>
#include <crow/gates/udpgate.h>
#include <crow/nodes/service_node.h>
#include <crow/tower.h>

#include <getopt.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

bool debug_mode = false;
int udpport = 0;
size_t chunk_size = 64;
std::string crowker_addr_str = ".12.127.0.0.1:10009";
std::string theme = "test_service";

crow::service_node service;

// Generate test response of specified size
std::string generate_response(size_t size)
{
    std::string data;
    data.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        data.push_back(static_cast<char>('A' + (i % 26)));
    }
    return data;
}

void service_handler(char *data, int len, crow::service_node &srv)
{
    std::string request(data, len);

    if (debug_mode)
    {
        fprintf(stderr, "[service] received request: '%s' (len=%d)\n",
                request.c_str(), len);
    }

    // Parse request: expecting "SIZE" or "SIZE:PATTERN"
    // Trim whitespace/newlines
    while (!request.empty() && (request.back() == '\n' || request.back() == '\r' || request.back() == ' '))
    {
        request.pop_back();
    }

    size_t response_size = 0;
    char pattern = 'A';

    if (request.find(':') != std::string::npos)
    {
        // FORMAT: SIZE:PATTERN
        size_t pos = request.find(':');
        response_size = std::stoul(request.substr(0, pos));
        if (pos + 1 < request.size())
        {
            pattern = request[pos + 1];
        }
    }
    else
    {
        // FORMAT: SIZE
        response_size = std::stoul(request);
    }

    // Generate response
    std::string response;
    response.reserve(response_size);
    for (size_t i = 0; i < response_size; ++i)
    {
        response.push_back(static_cast<char>(pattern + (i % 26)));
    }

    if (debug_mode)
    {
        fprintf(stderr, "[service] sending response: size=%zu, chunk_size=%zu\n",
                response.size(), srv.chunk_size());
    }

    srv.reply(response.data(), response.size());

    if (debug_mode)
    {
        fprintf(stderr, "[service] reply sent\n");
    }
}

void signal_handler(int)
{
    crow::stop_spin(false);
}

void print_help()
{
    printf(
        "Usage: large_reply_service [OPTIONS]\n"
        "\n"
        "Test service that replies with large chunked messages.\n"
        "\n"
        "Options:\n"
        "  -u, --udp PORT       UDP port (default: 0 = system assigned)\n"
        "  -c, --chunk SIZE     Chunk size for replies (default: 64)\n"
        "  -a, --crowker ADDR   Crowker address (default: .12.127.0.0.1:10009)\n"
        "  -t, --theme NAME     Service theme (default: test_service)\n"
        "  -d, --debug          Enable debug output\n"
        "  -h, --help           Show this help\n"
        "\n"
        "Request format:\n"
        "  SIZE           - respond with SIZE bytes (pattern A-Z repeating)\n"
        "  SIZE:CHAR      - respond with SIZE bytes starting from CHAR\n"
        "\n"
        "Example:\n"
        "  large_reply_service --chunk 100 --theme myservice\n"
        "  # Then from ctrans:\n"
        "  echo '500' | ctrans --request myservice\n"
    );
}

int main(int argc, char *argv[])
{
    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"udp", required_argument, NULL, 'u'},
        {"chunk", required_argument, NULL, 'c'},
        {"crowker", required_argument, NULL, 'a'},
        {"theme", required_argument, NULL, 't'},
        {"debug", no_argument, NULL, 'd'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hu:c:a:t:d", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help();
                return 0;
            case 'u':
                udpport = atoi(optarg);
                break;
            case 'c':
                chunk_size = std::stoul(optarg);
                break;
            case 'a':
                crowker_addr_str = optarg;
                break;
            case 't':
                theme = optarg;
                break;
            case 'd':
                debug_mode = true;
                crow::enable_diagnostic();
                break;
            default:
                return 1;
        }
    }

    // Create UDP gate
    if (crow::create_udpgate(12, udpport))
    {
        perror("udpgate");
        return 1;
    }

    // Parse crowker address
    auto crowker_addr = crow::address(crowker_addr_str);
    if (crowker_addr.size() == 0)
    {
        fprintf(stderr, "Invalid crowker address: %s\n", crowker_addr_str.c_str());
        return 1;
    }

    // Setup service node
    service.bind(50);  // arbitrary node id
    service.set_chunk_size(chunk_size);
    service.set_handle(service_handler);
    service.init_subscribe(crowker_addr,
                           CROWKER_SERVICE_BROCKER_NODE_NO,
                           theme.c_str(),
                           2,   // qos
                           50,  // ackquant
                           2,   // rqos
                           50); // rackquant
    service.install_keepalive(60000);  // very long keepalive to avoid re-subscribe during tests

    if (debug_mode)
    {
        fprintf(stderr, "[service] started: theme='%s', chunk_size=%zu, crowker='%s'\n",
                theme.c_str(), chunk_size, crowker_addr_str.c_str());
    }
    else
    {
        printf("Service ready: theme='%s', chunk_size=%zu\n", theme.c_str(), chunk_size);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    crow::spin_with_select();

    return 0;
}
