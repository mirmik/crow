#include <crow/address.h>
#include <crow/brocker/crowker_api.h>
#include <crow/brocker/crowker_pubsub_node.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <map>
#include <string>
#include <thread>
#include <random>

#include "control_node.h"
#include <crow/brocker/crowker.h>
#include <crow/pubsub/pubsub.h>

#include <igris/util/dstring.h>
#include <nos/fprint.h>
#include <nos/print.h>
#include <nos/io/stdfile.h>

#include <nos/inet/tcp_client.h>
#include <nos/inet/tcp_server.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

bool brocker_info = false;
int udpport = -1;
int tcpport = -1;
bool quite = false;
bool debug_mode = false;

const std::string VERSION = "2.1.0";

crow::Tower tower;
crow::crowker_pubsub_node pubsub_node;

void tcp_client_listener(nos::inet::tcp_client client)
{
    char buf[1024];
    nos::inet::netaddr addr = client.getaddr();

    if (brocker_info)
        nos::println_to(nos::cerr, "new tcp client from", addr);

    while (1)
    {
        nos::expected<long unsigned int, nos::input_error> ret;
        char cmd;
        uint32_t datasize;
        uint16_t thmsize;

        std::string theme;

        ret = client.recv(buf, 3, MSG_WAITALL);

        if (ret.is_error())
            break;

        cmd = buf[0];
        if (cmd != 's' && cmd != 'p')
            goto clean;

        buf[3] = 0;
        thmsize = strtol(buf + 1, nullptr, 10);

        if (thmsize == 0)
            goto clean;

        ret = client.recv(buf, thmsize, MSG_WAITALL);

        if (ret.is_error())
            break;

        theme = std::string(buf, thmsize);

        if (cmd == 's')
        {
            crow::crowker::instance()->tcp_subscribe(theme, &client);
            continue;
        }
        else if (cmd == 'p')
        {
            ret = client.recv(buf, 6, MSG_WAITALL);
            if (ret.is_error())
                break;

            buf[6] = 0;
            datasize = strtol(buf, nullptr, 10);
            if (datasize == 0)
                goto clean;

            ret = client.recv(buf, datasize, MSG_WAITALL);
            if (ret.is_error())
                break;

            auto data = std::make_shared<std::string>(buf, datasize);

            crow::crowker::instance()->publish(theme, data);
            continue;
        }

    clean:
    {
        if (brocker_info)
            nos::println_to(nos::cerr, "unresolved tcp command from", addr, cmd);

        continue;
    }
    }

    if (brocker_info)
        nos::println_to(nos::cerr, "tcp connection was clossed", addr);
}

void tcp_listener(int port)
{
    nos::inet::tcp_server srv;
    srv.init();
    srv.reusing(true);
    srv.bind("0.0.0.0", port);
    srv.listen(10);

    while (1)
    {
        auto client = srv.accept();

        std::thread thr(tcp_client_listener, std::move(client));
        thr.detach();
    }
}

void print_help()
{
    printf("Usage: crowker [OPTION]...\n"
           "\n"
           "Common option list:\n"
           "  -h, --help            print this page\n"
           "  -d, --debug           enable debug mode\n"
           "  -b, --binfo           enable info mode\n"
           "\n"
           "Gate`s option list:\n"
           "  -u, --udp             set udp address (gate 12)\n"
           "  -S, --serial          make gate on serial device\n"
           "\n");
}

int main(int argc, char *argv[])
{
    // Initialize with random seqid to reduce collision probability
    // after restart (TIME_WAIT entries on remote nodes still reference old seqids)
    std::random_device rd;
    tower.set_initial_seqid(rd() & 0xFFFF);

    // Set diagnostic label with PID for debugging
    tower.set_diagnostic_label(nos::format("crowker:{}", getpid()));

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    crow::pubsub_protocol.enable_crowker_subsystem();
#endif
    pubsub_node.bind(tower, CROWKER_SERVICE_BROCKER_NODE_NO);
    crow::crowker::instance()->add_api(&pubsub_node);

    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"udp", required_argument, NULL, 'u'}, // crow udpgate port
        {"tcp", required_argument, NULL, 't'},
        {"debug", no_argument, NULL, 'd'}, // crow transport log
        {"binfo", no_argument, NULL, 'b'}, // brocker log
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}};

    int long_index = 0;
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "u:t:svdib", long_options,
                              &long_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help();
                exit(0);

            case 'u':
                udpport = atoi(optarg);
                break;

            case 't':
                tcpport = atoi(optarg);
                break;

            case 'd':
                debug_mode = 1;
                tower.enable_diagnostic();
                break;

            case 'b':
                brocker_info = true;
                crow::crowker::instance()->brocker_info = true;
                break;

            case 'v':
                nos::println(VERSION);
                exit(0);
                break;

            case 0:
                break;
        }
    }

    if (udpport == -1)
    {
        if (debug_mode)
            fprintf(stderr, "Use default udp port 10009.\n");
        udpport = 10009;
    }

    auto udpgate = crow::create_udpgate_safe(CROW_UDPGATE_NO, udpport);
    if (!udpgate || !udpgate->opened())
    {
        perror("udpgate open");
        exit(-1);
    }
    udpgate->bind(tower, CROW_UDPGATE_NO);

    if (tcpport != -1)
    {
        std::thread thr(tcp_listener, tcpport);
        thr.detach();
    }

    init_control_node(tower);
    crow::spin_with_select(tower);
}
