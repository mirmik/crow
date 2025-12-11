#include <crow/address.h>
#include <crow/gates/serial_gstuff.h>
#include <crow/gates/tcpgate.h>
#include <crow/gates/udpgate.h>
#include <crow/nodes/beam.h>
#include <crow/nodes/node_delegate.h>
#include <crow/nodes/publisher_node.h>
#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/requestor_node.h>
#include <crow/nodes/service_node.h>
#include <crow/nodes/subscriber_node.h>
#include <crow/proto/acceptor.h>
#include <crow/proto/channel.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/asyncio.h>
#include <igris/event/delegate.h>
#include <fcntl.h>
#include <getopt.h>
#include <igris/osutil/timeouted_read.h>
#include <igris/util/bug.h>
#include <igris/util/dstring.h>
#include <igris/util/string.h>
#include <iostream>
#include <map>
#include <nos/fprint.h>
#include <nos/io/stdfile.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include <random>

const std::string VERSION = "2.1.0";

bool debug_mode = false;
crow::hostaddr address;
volatile bool cancel_token = false;

bool userqos = false;
uint8_t qos = 0;
uint8_t type = 0;
uint16_t ackquant = 200;
unsigned int latest = 0;

bool api = false;
bool noconsole = false;
bool nlout = false;
bool noend = false;
bool echo = false;
bool gdebug = false;
bool info = false;
bool beam_mode = false;
bool subscribe_mode = false;
bool latest_mode = false;
bool service_mode = false;
bool request_mode = false;
bool crowker_mode = false;
bool exit_on_receive = false;
bool crowker_control_request_mode = false;
bool TIMESTAMP_MODE = false;

int acceptorno = -1;
int channelno = -1;
int nodeno = -1;
std::string nodename = "";
std::string crowker_control_request_command;

crow::channel channel;
crow::channel *reverse_channel;
crow::acceptor acceptor;

std::vector<int> listened_nodes;
std::string pulse;
std::string theme;
std::string pipelinecmd;
std::string reply_theme = ":unchanged:";

int DATAOUTPUT_FILENO = STDOUT_FILENO;
int DATAINPUT_FILENO = STDIN_FILENO;

crow::Tower tower;
std::shared_ptr<crow::udpgate> udpgate;
std::shared_ptr<crow::tcpgate> tcpgate;

constexpr uint8_t CTRANS_SERIAL_GATE_NO = 42;
constexpr const char *CTRANS_HEADER_V0 = "crow::header_v0";
constexpr const char *CTRANS_HEADER_V1 = "crow::header_v1";
constexpr const char *CTRANS_GSTUFF_V0 = "gstuff_v0";
constexpr const char *CTRANS_GSTUFF_V1 = "gstuff_v1";

struct gate_info
{
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;
};

std::vector<gate_info> created_gate_info;

void register_gate_info(const std::string &name,
                        std::vector<std::pair<std::string, std::string>> params)
{
    gate_info info;
    info.name = name;
    info.parameters = std::move(params);
    created_gate_info.emplace_back(std::move(info));
}

std::string parity_description(char parity)
{
    switch (parity)
    {
        case 'n':
        case 'N':
            return "none";

        case 'e':
        case 'E':
            return "even";

        case 'o':
        case 'O':
            return "odd";

        default:
            break;
    }

    return "unknown";
}

std::string format_parity_value(char parity)
{
    std::string result(1, parity);
    result += " (" + parity_description(parity) + ")";
    return result;
}

std::string describe_port(uint16_t port)
{
    if (port == 0)
    {
        return "0 (system-assigned)";
    }

    return std::to_string(port);
}

void register_serial_gate_info(const std::string &path, int baud, char parity,
                               int stopbits, int databits,
                               const char *header_version,
                               const char *gstuff_version)
{
    register_gate_info("serial_gstuff",
                       {{"gate_no", std::to_string(CTRANS_SERIAL_GATE_NO)},
                        {"device", path},
                        {"baud", std::to_string(baud)},
                        {"parity", format_parity_value(parity)},
                        {"stopbits", std::to_string(stopbits)},
                        {"databits", std::to_string(databits)},
                        {"header", header_version},
                        {"gstuff", gstuff_version}});
}

void do_incom_data(nos::buffer);

void subscriber_data_handle(nos::buffer incom_data)
{
    do_incom_data(incom_data);
}

int service_data_handle(char *cmd, int cmdlen, char *ans, int ansmax)
{
    do_incom_data({cmd, (size_t)cmdlen});
    snprintf(ans, ansmax, "HelloWorld");
    return strlen(ans);
}

void requestor_data_handle(nos::buffer incom_data)
{
    if (debug_mode)
    {
        fprintf(stderr, "requestor_data_handle: size=%zu, last_byte=0x%02x\n",
                incom_data.size(),
                incom_data.size() > 0 ? (uint8_t)incom_data.data()[incom_data.size()-1] : 0);
    }
    do_incom_data(incom_data);
}

void service_data_handle(char *data, int len, crow::service_node &)
{
    do_incom_data({data, (size_t)len});
}

void raw_node_incom_handle(crow::node_packet_ptr incom_data)
{
    do_incom_data(incom_data.message());
}

void raw_node_undel_handle(crow::node_packet_ptr incom_data)
{
    (void)incom_data;
}

crow::node_delegate raw_node(raw_node_incom_handle, raw_node_undel_handle);
crow::publisher_node publish_node;
crow::requestor_node requestor_node(requestor_data_handle);
crow::subscriber_node subscriber_node(subscriber_data_handle);
crow::service_node service_node(service_data_handle);
crow::beam beam;

enum class protoopt_e
{
    PROTOOPT_BASIC,
    PROTOOPT_CHANNEL,
    PROTOOPT_NODE,
    PROTOOPT_REVERSE_CHANNEL,
    PROTOOPT_REQUEST,
    PROTOOPT_PUBLISH_NODE
};

enum class input_format
{
    INPUT_RAW,
    INPUT_RAW_ENDLINE,
};

enum class output_format
{
    OUTPUT_RAW,
    OUTPUT_DSTRING,
};

protoopt_e protoopt = protoopt_e::PROTOOPT_BASIC;
input_format informat = input_format::INPUT_RAW;
output_format outformat = output_format::OUTPUT_RAW;

std::string informat_tostr()
{
    switch (informat)
    {
        case input_format::INPUT_RAW:
            return "INPUT_RAW";

        case input_format::INPUT_RAW_ENDLINE:
            return "INPUT_RAW_ENDLINE";

        default:
            BUG();
    }

    return std::string();
}

std::string outformat_tostr()
{
    switch (outformat)
    {
        case output_format::OUTPUT_RAW:
            return "OUTPUT_RAW";

        case output_format::OUTPUT_DSTRING:
            return "OUTPUT_DSTRING";

        default:
            BUG();
    }

    return std::string();
}

std::string gen_random_string(const int len)
{
    std::string tmp_s;
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

    srand((unsigned)time(NULL) * getpid());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    return tmp_s;
}

void output_do(nos::buffer data, crow::packet *pack)
{
    int ret;
    (void)pack;

    if (TIMESTAMP_MODE)
    {
        auto time = igris::millis();
        auto str = std::to_string(time);
        int _ = write(DATAOUTPUT_FILENO, str.data(), str.size());
        (void)_;
    }

    switch (outformat)
    {
        case output_format::OUTPUT_RAW:
            ret = write(DATAOUTPUT_FILENO, data.data(), data.size());
            break;

        case output_format::OUTPUT_DSTRING:
            // Вывод в stdout информацию пакета.
            char buf[10000];
            bytes_to_dstring(buf, data.data(), data.size());
            ret = write(DATAOUTPUT_FILENO, buf, strlen(buf));
            break;

        default:
            BUG();
    }

    if (nlout)
    {
        ret = write(DATAOUTPUT_FILENO, "\n", 1);
    }

    if (ret < 0)
        exit(0);
}

void do_incom_data(nos::buffer incom_data)
{
    output_do(incom_data, nullptr);
    if (exit_on_receive)
    {
        crow::asyncio.cancel();
        cancel_token = true;
    }
}

std::pair<std::string, bool> input_do(const std::string &data)
{
    std::string message;

    switch (informat)
    {
        case input_format::INPUT_RAW_ENDLINE:
            message = data;
            message += '\n';
            return std::make_pair(message, true);

        case input_format::INPUT_RAW:
            message = data;
            return std::make_pair(message, true);

        default:
            BUG();
    }
}

void pipeline(const std::string &cmd)
{
    int ans;

    int child_pid;
    int pipe_to_child[2];
    int pipe_from_child[2];

    char *child_args[] = {(char *)cmd.c_str(), (char *)0};

    ans = pipe(pipe_to_child);

    if (ans)
    {
        perror("pipe");
        exit(-1);
    }

    ans = pipe(pipe_from_child);

    if (ans)
    {
        perror("pipe");
        exit(-1);
    }

    if ((child_pid = fork()) == 0)
    {
        // child branch
        dprln("child:", getpid());

        dup2(pipe_from_child[1], STDOUT_FILENO);
        dup2(pipe_to_child[0], STDIN_FILENO);
        close(pipe_from_child[0]);
        close(pipe_to_child[1]);

        ans = execvp(cmd.c_str(), child_args);

        if (ans)
        {
            perror("execve");
            exit(-1);
        }
    }

    DATAINPUT_FILENO = pipe_from_child[0];
    DATAOUTPUT_FILENO = pipe_to_child[1];
    close(pipe_from_child[1]);
    close(pipe_to_child[0]);
}

void send_do(const std::string message)
{
    switch (protoopt)
    {
        case protoopt_e::PROTOOPT_BASIC:
            tower.send(
                address, {message.data(), message.size()}, type, qos, ackquant);
            break;

        case protoopt_e::PROTOOPT_PUBLISH_NODE:
        {
            publish_node.publish(address,
                                 CROWKER_SERVICE_BROCKER_NODE_NO,
                                 theme.c_str(),
                                 message,
                                 qos,
                                 ackquant);
        }
        break;

        case protoopt_e::PROTOOPT_CHANNEL:
        {
            int ret = channel.send(message.data(), message.size());

            if (ret == CROW_CHANNEL_ERR_NOCONNECT)
            {
                nos::println_to(nos::cerr, "Channel is not connected");
            }
        }
        break;

        case protoopt_e::PROTOOPT_NODE:
        {
            crow::node::send(1,
                             nodeno,
                             address,
                             {message.data(), message.size()},
                             qos,
                             ackquant);
        }
        break;

        case protoopt_e::PROTOOPT_REQUEST:
        {
            requestor_node.async_request(address,
                                         CROWKER_SERVICE_BROCKER_NODE_NO,
                                         theme,
                                         reply_theme,
                                         {message.data(), message.size()},
                                         qos,
                                         ackquant,
                                         qos,
                                         ackquant);
        }
        break;

        case protoopt_e::PROTOOPT_REVERSE_CHANNEL:
        {
            int ret = reverse_channel->send(message.data(), message.size());

            if (ret == CROW_CHANNEL_ERR_NOCONNECT)
            {
                nos::println_to(nos::cerr, "Channel is not connected");
            }
        }
        break;
    }
}

void incoming_handler(crow::packet *pack)
{
    if (echo)
    {
        // Переотослать пакет точно повторяющий входящий.
        tower.send(pack->addr(),
                   {pack->dataptr(), pack->datasize()},
                   pack->type(),
                   pack->quality(),
                   pack->ackquant());
    }

    if (api)
    {
        // Запуск встроенных функций.
        char *dp = pack->dataptr();
        size_t ds = pack->datasize();

        if (strncmp(dp, "remote_exit\n", ds) == 0)
        {
            quick_exit(0);
        }
    }

    output_do(pack->data(), pack);
    crow::Tower::release(pack);
}

void print_channel_message(crow::channel *ch, crow::packet *pack)
{
    (void)ch;
    (void)pack;
    output_do(crow::channel::getdata(pack), pack);
}

crow::channel *acceptor_create_channel()
{
    crow::channel *ch = new crow::channel(print_channel_message);

    // TODO: Утечка памяти. Удалять буффер на закрытии канала.
    ch->set_addr_buffer((char *)malloc(128), 128);

    reverse_channel = ch;
    return ch;
}

void console_read_handler(int fd)
{
    char readbuf[1024];
    int len = read(fd, readbuf, 1024);

    if (len <= 0)
    {
        if (len == 0)
        {
            // EOF on stdin - stop the event loop
            crow::asyncio.cancel();
            cancel_token = true;
        }
        // len < 0 means EAGAIN/EWOULDBLOCK for non-blocking fd, just return
        return;
    }

    std::string input(readbuf, len);
    auto msgpair = input_do(input);

    if (msgpair.second)
        send_do(msgpair.first);
}

uint16_t udpport = 0;
uint16_t tcpport = 0;
char *serial_port = NULL;

void print_help()
{
    printf(
        "Usage: ctrans [OPTION]... ADDRESS\n"
        "\n"
        "Common option list:\n"
        "  -h, --help            print this page\n"
        "\n"
        "Gate`s option list:\n"
        "  -u, --udp             set udp address (gate 12)\n"
        "  -c, --tcp             set tcp address (gate 13)\n"
        "  -S, --serial          make gate on serial device.\n"
        "                        Format:\n"
        "                          "
        "PATH[:BAUD[:PARITY[:STOPBITS[:DATABITS[:v0|v1[:g0|g1]]]]]]\n"
        "                          Header version v0 or v1 (default v1)\n"
        "                          Gstuff version g0 or g1 (default g1)\n"
        "                        Defaults: 115200:n:1:8:v1:g1, parity n|e|o\n"
        "\n"
        "Package settings option list:\n"
        "  -q, --qos             set QOS policy mode\n"
        "  -A, --ackquant        set time quant (for QOS:1 and QOS:2)\n"
        "  -t, --type            set package type (if protocol isn't choosen)\n"
        "\n"
        "Protocol option list:\n"
        "      --chlisten        (channel) enable acceptor mode\n"
        "      --channel         (channel) connect to channel on nid\n"
        "      --node            (node)    send message to specified node\n"
        "      --listen-node     (node)    listen that node ids\n"
        "      --request         (node)      request crowker service\n"
        "      --subscribe       (node)    subscribe to crowker theme\n"
        "      --publish         (node)    publish to crowker theme\n"
        "      --beam            (node)    \n"
        "      --control         (node)    link to crowker control cli\n"
        "\n"
        "Info option list:\n"
        "      --info\n"
        "      --debug\n"
        "      --dumpsize\n"
        "      --gdebug\n"
        "      --timestamp\n"
        "\n"
        "Control option list:\n"
        "      --noconsole       disable console input\n"
        "      --pulse           oneshoot mode. leave after first message\n"
        "      --echo            echo input packages to sender\n"
        "      --api             enable incoming console (cmds: 'exit')\n"
        "      --retransler      enable retransling option\n"
        "\n"
        "Crow address format:\n"
        "        A crow address is a byte sequence describing a routing path:\n"
        "        [gate_no][gate_arg][next_gate_no][next_gate_arg]...\n"
        "        Gate argument length is arbitrary and may be empty.\n"
        "   \n"
        "Hexer syntax used in ADDRESS:\n"
        "        .N        decimal byte (0..255), e.g. .12 .127\n"
        "        :N        decimal 16-bit value (big-endian), usually a port, "
        "e.g. :10009\n"
        "        #HEX raw hexadecimal bytes, e.g.#0C7F0000012714\n "
        "        @NAME     ASCII letters written as-is, e.g. @UDP\n"
        "        _         visual separator, ignored by parser\n"
        "\n"
        "Examples:\n"
        "       .12.127.0.0.1:10009\n"
        "       UDP gate 12 → 127.0.0.1:10009\n"
        "       #F2 .12.127.0.0.1 : 10009\n "
        "       Adds prefix byte F2 before the UDP gate\n");
}

void signal_handler(int)
{
    quick_exit(0);
}

void parse_options(int argc, char **argv)
{
    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},

        {"udp", required_argument, NULL, 'u'}, // udp порт для 12-ого гейта.
        {"tcp", required_argument, NULL, 'c'}, // udp порт для 12-ого гейта.
        {"serial", required_argument, NULL, 'S'}, // serial...

        {"qos",
         required_argument,
         NULL,
         'q'}, // qos отправляемых сообщений. 0 по умолчанию
        {"type",
         required_argument,
         NULL,
         't'}, // метка типа отправляемых сообщений
        {"ackquant", required_argument, NULL, 'A'}, // установка кванта ack

        {"noend",
         no_argument,
         NULL,
         'x'}, // Блокирует добавление символа конца строки.
        {"nlout",
         no_argument,
         NULL,
         'N'}, // Блокирует добавление символа конца строки.
        {"echo",
         no_argument,
         NULL,
         'E'}, // Активирует функцию эха входящих пакетов.
        {"api", no_argument, NULL, 'a'}, // Активирует удалённое управление.
        {"noconsole", no_argument, NULL, 'n'}, // Отключает создание консоли.
        {"pulse",
         required_argument,
         NULL,
         'p'}, // Отключает программу по первой транзакции.
        {"timestamp",
         no_argument,
         NULL,
         'T'}, // Выводит метку времени прешедшего пакета.

        {"rawout", no_argument, NULL, 'r'},
        {"dbgout", no_argument, NULL, 'j'},

        {"chlisten", required_argument, NULL, 'w'},
        {"channel", required_argument, NULL, 'H'},
        {"node", required_argument, NULL, 'M'},
        {"listen-node", required_argument, NULL, 'U'},
        {"pipeline", required_argument, NULL, 'e'},

        {"subscribe", required_argument, NULL, 'K'},
        {"latest", required_argument, NULL, 'D'},
        {"publish", required_argument, NULL, 'L'},
        {"request", required_argument, NULL, 'G'},
        {"service", required_argument, NULL, 'Y'},
        {"crowcli", required_argument, NULL, 'm'},
        {"beam", required_argument, NULL, 'b'},
        {"retransler", no_argument, NULL, 'R'},
        {"control", no_argument, NULL, 'Q'},
        {"version", no_argument, NULL, 'V'},

        {"info",
         no_argument,
         NULL,
         'i'}, // Выводит информацию о имеющихся гейтах и режимах.
        {"debug",
         no_argument,
         NULL,
         'd'}, // Включает информацию о событиях башни.
        {"dumpsize",
         required_argument,
         NULL,
         's'}, // Включает информацию о событиях башни.
        {"gdebug", no_argument, NULL, 'g'}, // Активирует информацию о вратах.
        {NULL, 0, NULL, 0}};

    int long_index = 0;
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help();
                exit(0);

            case 'q':
                qos = (uint8_t)atoi(optarg);
                userqos = true;
                break;

            case 'A':
                ackquant = (uint16_t)atoi(optarg);
                break;

            case 's':
                tower.set_debug_data_size((uint16_t)atoi(optarg));
                break;

            case 't':
                type = (uint8_t)atoi(optarg);
                break;

            case 'T':
                TIMESTAMP_MODE = true;
                break;

            case 'u':
                udpport = (uint16_t)atoi(optarg);
                break;

            case 'c':
                tcpport = (uint16_t)atoi(optarg);
                break;

            case 'S':
                serial_port = (char *)malloc(strlen(optarg) + 1);
                strcpy(serial_port, optarg);
                break;

            case 'E':
                echo = true;
                break;

            case 'x':
                noend = true;
                break;

            case 'N':
                nlout = true;
                break;

            case 'i':
                info = true;
                break;

            case 'Q':
                crowker_mode = true;
                protoopt = protoopt_e::PROTOOPT_NODE;
                nodeno = CROWKER_CONTROL_BROCKER_NODE_NO;
                break;

            case 'R':
                tower.set_retransling_allowed(true);
                break;

            case 'r':
                outformat = output_format::OUTPUT_RAW;
                break;

            case 'j':
                outformat = output_format::OUTPUT_DSTRING;
                break;

            case 'n':
                noconsole = true;
                break;

            case 'U':
            {
                auto lst = igris::split(optarg, ',');
                for (auto a : lst)
                {
                    listened_nodes.push_back(atoi(a.data()));
                }
            }
            break;

            case 'g':
                gdebug = true;
                break;

            case 'p':
                pulse = optarg;
                break;

            case 'a':
                api = true;
                break;

            case 'd':
                debug_mode = true;
                tower.enable_diagnostic();
                break;

            case 'L':
                theme = optarg;
                protoopt = protoopt_e::PROTOOPT_PUBLISH_NODE;
                crowker_mode = 1;
                break;

            case 'Y':
                theme = optarg;
                service_mode = 1;
                crowker_mode = 1;
                break;

            case 'K':
                theme = optarg;
                subscribe_mode = 1;
                crowker_mode = 1;
                break;

            case 'D':
                latest = atoi(optarg);
                latest_mode = 1;
                break;

            case 'G':
                theme = optarg;
                request_mode = 1;
                protoopt = protoopt_e::PROTOOPT_REQUEST;
                crowker_mode = 1;
                break;

            case 'm':
                crowker_mode = 1;
                crowker_control_request_mode = 1;
                crowker_control_request_command = optarg;
                exit_on_receive = 1;
                noconsole = 1;
                break;

            case 'b':
                beam_mode = 1;
                noconsole = 1;
                crowker_mode = 1;
                beam.set_client_name(optarg);
                break;

            case 'w':
                acceptorno = atoi(optarg);
                protoopt = protoopt_e::PROTOOPT_REVERSE_CHANNEL;
                break;

            case 'H':
                channelno = atoi(optarg);
                protoopt = protoopt_e::PROTOOPT_CHANNEL;
                break;

            case 'M':
                if (isalpha(*optarg))
                    nodename = optarg;
                else
                    nodeno = atoi(optarg);

                protoopt = protoopt_e::PROTOOPT_NODE;
                break;

            case 'e':
                pipelinecmd = optarg;
                break;

            case 'V':
                nos::println_to(nos::cerr, VERSION);
                exit(0);
                break;

            case '?':
                exit(-1);
                break;

            case 0:
                nos::println_to(nos::cerr, "getopt error");
                exit(-1);
                break;
        }
    }
}

void create_serial_gate_v0(std::string path, int baud, char parity,
                           int stopbits, int databits, bool gstuff_v1)
{
    gstuff_context gctx = gstuff_v1 ? gstuff_context{} : gstuff_context_v0();
    crow::serial_gstuff_v0 *gate = nullptr;
    if ((gate = crow::create_serial_gstuff_v0(path.c_str(),
                                              115200,
                                              CTRANS_SERIAL_GATE_NO,
                                              gdebug,
                                              gctx)) == NULL)
    {
        perror("serialgate open");
        exit(-1);
    }
    if (gate)
    {
        gate->setup_serial_port(baud, parity, stopbits, databits);
        register_serial_gate_info(
            path, baud, parity, stopbits, databits, CTRANS_HEADER_V0,
            gstuff_v1 ? CTRANS_GSTUFF_V1 : CTRANS_GSTUFF_V0);
    }
}

void create_serial_gate_v1(std::string path, int baud, char parity,
                           int stopbits, int databits, bool gstuff_v1)
{
    gstuff_context gctx = gstuff_v1 ? gstuff_context{} : gstuff_context_v0();
    crow::serial_gstuff *gate = nullptr;
    if ((gate = crow::create_serial_gstuff(path.c_str(),
                                           115200,
                                           CTRANS_SERIAL_GATE_NO,
                                           gdebug,
                                           gctx)) == NULL)
    {
        perror("serialgate open");
        exit(-1);
    }
    if (gate)
    {
        gate->setup_serial_port(baud, parity, stopbits, databits);
        register_serial_gate_info(
            path, baud, parity, stopbits, databits, CTRANS_HEADER_V1,
            gstuff_v1 ? CTRANS_GSTUFF_V1 : CTRANS_GSTUFF_V0);
    }
}

void create_serial_gate_v0(std::string path)
{
    create_serial_gate_v0(path, 115200, 'n', 1, 8, true);
}

void create_serial_gate_v1(std::string path)
{
    create_serial_gate_v1(path, 115200, 'n', 1, 8, true);
}

static bool parse_gstuff_version(const std::string &token)
{
    if (token == "g0")
        return false;
    if (token == "g1")
        return true;
    std::cerr << "Unknown gstuff version: " << token << " (expected g0 or g1)" << std::endl;
    exit(-1);
}
void create_serial_gate(std::vector<std::string> tokens)
{
    // PATH
    if (tokens.size() == 1)
    {
        create_serial_gate_v1(tokens[0]);
    }
    // PATH:BAUD:PARITY:STOPBITS:DATABITS
    else if (tokens.size() == 5)
    {
        create_serial_gate_v1(tokens[0],
                              std::stoi(tokens[1]),
                              tokens[2][0],
                              std::stoi(tokens[3]),
                              std::stoi(tokens[4]),
                              true);
    }
    // PATH:BAUD:PARITY:STOPBITS:DATABITS:v0|v1
    else if (tokens.size() == 6)
    {
        bool gstuff_v1 = true;

        if (tokens[5] == "v0")
        {
            nos::println_to(nos::cerr, "create_serial_gate header=v0 gstuff=v1");
            create_serial_gate_v0(tokens[0],
                                  std::stoi(tokens[1]),
                                  tokens[2][0],
                                  std::stoi(tokens[3]),
                                  std::stoi(tokens[4]),
                                  gstuff_v1);
        }
        else if (tokens[5] == "v1")
        {
            nos::println_to(nos::cerr, "create_serial_gate header=v1 gstuff=v1");
            create_serial_gate_v1(tokens[0],
                                  std::stoi(tokens[1]),
                                  tokens[2][0],
                                  std::stoi(tokens[3]),
                                  std::stoi(tokens[4]),
                                  gstuff_v1);
        }
        else
        {
            std::cerr << "Unknown header version: " << tokens[5] << std::endl;
            exit(-1);
        }
    }
    // PATH:BAUD:PARITY:STOPBITS:DATABITS:v0|v1:g0|g1
    else if (tokens.size() == 7)
    {
        bool gstuff_v1 = parse_gstuff_version(tokens[6]);

        if (tokens[5] == "v0")
        {
            nos::println_to(nos::cerr, "create_serial_gate header=v0 gstuff=", gstuff_v1 ? "v1" : "v0");
            create_serial_gate_v0(tokens[0],
                                  std::stoi(tokens[1]),
                                  tokens[2][0],
                                  std::stoi(tokens[3]),
                                  std::stoi(tokens[4]),
                                  gstuff_v1);
        }
        else if (tokens[5] == "v1")
        {
            nos::println_to(nos::cerr, "create_serial_gate header=v1 gstuff=", gstuff_v1 ? "v1" : "v0");
            create_serial_gate_v1(tokens[0],
                                  std::stoi(tokens[1]),
                                  tokens[2][0],
                                  std::stoi(tokens[3]),
                                  std::stoi(tokens[4]),
                                  gstuff_v1);
        }
        else
        {
            std::cerr << "Unknown header version: " << tokens[5] << std::endl;
            exit(-1);
        }
    }
    else
    {
        std::cout << "Unknown serial port format. tokens: " << std::endl;
        for (auto &t : tokens)
        {
            std::cout << t << std::endl;
        }
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    // Initialize with random seqid to reduce collision probability
    // after restart (TIME_WAIT entries on remote nodes still reference old seqids)
    std::random_device rd;
    tower.set_initial_seqid(rd() & 0xFFFF);

    // Set diagnostic label with PID for debugging
    tower.set_diagnostic_label(nos::format("ctrans:{}", getpid()));

    parse_options(argc, argv);

    raw_node.bind(tower, 1);
    publish_node.bind(tower, CTRANS_DEFAULT_PUBLISHER_NODE);
    subscriber_node.bind(tower, CTRANS_DEFAULT_SUBSCRIBER_NODE);
    service_node.bind(tower, CTRANS_DEFAULT_SERVICE_NODE);
    requestor_node.bind(tower, CTRANS_DEFAULT_REQUESTOR_NODE);
    beam.bind(tower, CTRANS_DEFAULT_BEAM_NODE);
    reply_theme = gen_random_string(10);

    udpgate = crow::create_udpgate_safe(CROW_UDPGATE_NO, udpport);
    if (!udpgate->opened())
    {
        perror("udpgate open");
        exit(-1);
    }
    udpgate->bind(tower, CROW_UDPGATE_NO);
    register_gate_info("udpgate",
                       {{"gate_no", std::to_string(CROW_UDPGATE_NO)},
                        {"listen_port", describe_port(udpport)},
                        {"header", CTRANS_HEADER_V1}});

    if (tcpport)
    {
        tcpgate = crow::create_tcpgate_safe(CROW_TCPGATE_NO, tcpport);
        if (!tcpgate->opened())
        {
            perror("tcpgate open");
            exit(-1);
        }
        tcpgate->bind(tower, CROW_TCPGATE_NO);
        register_gate_info("tcpgate",
                           {{"gate_no", std::to_string(CROW_TCPGATE_NO)},
                            {"listen_port", describe_port(tcpport)},
                            {"header", CTRANS_HEADER_V1}});
    }

    if (serial_port != NULL)
    {
        auto tokens = igris::split(std::string(serial_port), ':');
        create_serial_gate(tokens);
    }

    // Переопределение стандартного обработчика (Для возможности перехвата и
    // api)
    tower.set_default_incoming_handler(incoming_handler);

    if (noend)
    {
        informat = input_format::INPUT_RAW;
    }

    // Определение целевого адреса
    if (optind < argc)
    {
        address = crow::address_warned(argv[optind]);
        if (address.size() == 0)
        {
            nos::println_to(nos::cerr, "address error");
            exit(0);
        }

        //		if (address.size() < 0)
        //		{
        //			printf("Wrong address format\n");
        //			exit(-1);
        //		}
    }
    else
    {
        if (crowker_mode)
        {
            address = crow::crowker_address();
        }
    }

    // Вывод информации о созданных вратах.
    if (info)
    {
        if (created_gate_info.empty())
        {
            nos::println_to(nos::cerr, "gates info: none");
        }
        else
        {
            nos::println_to(nos::cerr, "gates info:");
            for (const auto &gate : created_gate_info)
            {
                nos::fprintln_to(nos::cerr, "  {}", gate.name);
                for (const auto &param : gate.parameters)
                {
                    nos::fprintln_to(nos::cerr, "    {}: {}", param.first, param.second);
                }
            }
        }

        nos::fprintln_to(nos::cerr, "input format: {}", informat_tostr());
        nos::fprintln_to(nos::cerr, "output format: {}", outformat_tostr());
    }

    if (pipelinecmd != "")
    {
        pipeline(pipelinecmd);
    }

    signal(SIGINT, signal_handler);

    // Channel mode requires synchronous connect which needs threaded event loop
    // TODO: implement async channel connect
    if (channelno >= 0)
    {
        nos::println_to(nos::cerr, "Error: --channel mode is not yet supported in single-threaded async mode");
        exit(-1);
    }

    if (acceptorno >= 0)
    {
        acceptor.init(acceptorno, acceptor_create_channel);
    }

    // Ветка обработки pulse мода.
    if (pulse != "")
    {
        auto msgpair = input_do(pulse);

        if (msgpair.second)
            send_do(msgpair.first);

        // For request mode, wait for response before exiting
        if (request_mode)
        {
            exit_on_receive = true;
            // Fall through to spin loop to wait for response
        }
        else
        {
            // For non-request modes, run event loop until packets are sent
            while (tower.has_untravelled() || crow::has_allocated())
            {
                tower.onestep();
                crow::asyncio.step(tower.get_minimal_timeout());
            }
            exit(0);
        }
    }

    if (subscribe_mode)
    {
        subscriber_node.init_subscribe(address,
                                       CROWKER_SERVICE_BROCKER_NODE_NO,
                                       theme.c_str(),
                                       qos,
                                       ackquant,
                                       qos,
                                       ackquant);
        subscriber_node.install_keepalive(2000);
    }

    if (subscribe_mode && latest_mode)
    {
        subscriber_node.init_subscribe(address,
                                       CROWKER_SERVICE_BROCKER_NODE_NO,
                                       theme.c_str(),
                                       qos,
                                       ackquant,
                                       qos,
                                       ackquant);
        subscriber_node.subscribe_v2(true, latest);
        subscriber_node.install_keepalive(2000, false);
    }

    if (service_mode)
    {
        service_node.init_subscribe(address,
                                    CROWKER_SERVICE_BROCKER_NODE_NO,
                                    theme.c_str(),
                                    qos,
                                    ackquant,
                                    qos,
                                    ackquant);
        service_node.install_keepalive(2000);
    }

    if (crowker_control_request_mode)
    {
        raw_node.send(CROWKER_CONTROL_BROCKER_NODE_NO,
                      address,
                      crowker_control_request_command,
                      qos,
                      ackquant);
    }

    if (beam_mode)
    {
        beam.set_crowker_address(address);
        beam.install_keepalive(2000);
        beam.keepalive_handle();
    }

    // Register console input as async event source
    if (!noconsole)
    {
        nos::osutil::nonblock(DATAINPUT_FILENO, true);
        crow::asyncio.add_iotask(
            DATAINPUT_FILENO,
            crow::SelectType::READ,
            igris::make_delegate(console_read_handler));
    }

    // Run single-threaded event loop
    crow::spin_with_select(tower);
    quick_exit(0);
}
