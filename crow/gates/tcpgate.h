#ifndef CROW_GATES_TCPGATE_H
#define CROW_GATES_TCPGATE_H

#include <crow/gateway.h>
#include <nos/inet/tcp_server.h>
#include <nos/inet/tcp_socket.h>

#include <map>
#include <memory>

#define CROW_TCPGATE_NO 13

namespace crow
{
    class tcpgate : public gateway
    {
        nos::inet::tcp_server server;
        std::map<nos::inet::netaddr, nos::inet::tcp_socket> sockets;
        bool need_update_fds = true;

      public:
        tcpgate() {}
        tcpgate(uint16_t port) { open(port); }

        void send(crow::packet *) override;
        void nblock_onestep() override;

        bool opened() { return server.fd > 0; }

        int open(uint16_t port = 0);
        void close();
        void finish() override { close(); }

        int bind(int gate_no = CROW_TCPGATE_NO)
        {
            return gateway::bind(gate_no);
        }

        void open_new_client(nos::inet::netaddr addr)
        {
            auto sock = nos::inet::tcp_socket(addr);
            sockets[addr] = sock;
        }

#if CROW_ENABLE_WITHOUT_FDS
#else
        std::vector<int> get_fds() override
        {
            std::vector<int> fds;
            fds.reserve(1 + sockets.size());
            fds.push_back(server.fd);
            for (auto &p : sockets)
                fds.push_back(p.second.fd);
            need_update_fds = false;
            return fds;
        }
#endif

        ~tcpgate() override { close(); }
    };

    std::shared_ptr<crow::tcpgate> create_tcpgate_safe(uint8_t id,
                                                       uint16_t port = 0);
}

#endif