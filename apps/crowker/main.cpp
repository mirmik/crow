#include <crow/address.h>
#include <crow/brocker/crowker_api.h>
#include <crow/brocker/crowker_pubsub_node.h>
#include <crow/gates/tcpgate.h>
#include <crow/gates/udpgate.h>
#include <crow/proto/node_protocol.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <map>
#include <string>
#include <random>

#include "control_node.h"
#include "webui.h"
#include <crow/brocker/crowker.h>
#include <crow/pubsub/pubsub.h>

#include <igris/util/dstring.h>
#include <nos/fprint.h>
#include <nos/print.h>
#include <nos/io/stdfile.h>

#include <csignal>

void signal_handler(int signum)
{
    (void)signum;
    nos::println("\nReceived signal, shutting down...");
    crowker_webui::stop();
    crow::set_spin_cancel_token();
}

bool brocker_info = false;
int udpport = -1;
int tcpport = -1;
int httpport = -1;
bool quite = false;
bool debug_mode = false;

const std::string VERSION = "2.1.0";

crow::Tower tower;
crow::crowker_pubsub_node pubsub_node;

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
           "  -u, --udp             set udp port (gate 12)\n"
           "  -t, --tcp             set tcp port (gate 13)\n"
           "  -w, --http            set web UI http port (default: 8080)\n"
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
    // Enable chunking for large messages (UDP MTU is ~1400 bytes)
    pubsub_node.set_chunk_size(1000);
    crow::crowker::instance()->add_api(&pubsub_node);

    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"udp", required_argument, NULL, 'u'},
        {"tcp", required_argument, NULL, 't'},
        {"http", required_argument, NULL, 'w'},
        {"debug", no_argument, NULL, 'd'},
        {"binfo", no_argument, NULL, 'b'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}};

    int long_index = 0;
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "u:t:w:svdib", long_options,
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

            case 'w':
                httpport = atoi(optarg);
                break;

            case 'd':
                debug_mode = 1;
                tower.enable_diagnostic();
                crow::node_protocol.set_debug(true);
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

    // Create crow TCP gate if requested
    std::shared_ptr<crow::tcpgate> tcpgate;
    if (tcpport != -1)
    {
        tcpgate = crow::create_tcpgate_safe(CROW_TCPGATE_NO, tcpport);
        if (!tcpgate || !tcpgate->opened())
        {
            perror("tcpgate open");
            exit(-1);
        }
        tcpgate->bind(tower, CROW_TCPGATE_NO);
        if (debug_mode)
            tcpgate->debug(true);
        if (!quite)
            nos::fprintln("TCP gate listening on port {}", tcpport);
    }

    // Start web UI server
    if (httpport != -1)
    {
        if (crowker_webui::start(httpport, brocker_info))
        {
            if (!quite)
                nos::fprintln("Web UI started on http://0.0.0.0:{}", httpport);
        }
        else
        {
            nos::fprintln_to(nos::cerr, "Failed to start Web UI on port {}", httpport);
        }
    }

    init_control_node(tower);

    // Регистрируем обработчик сигнала для корректного завершения
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    crow::spin_with_select(tower);

    nos::println("Crowker stopped.");
}
