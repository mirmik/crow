/**
    @file
*/

#ifndef CROW_NODE_SPAMMER_H
#define CROW_NODE_SPAMMER_H

#include <chrono>
#include <crow/extra/nodeaddr.h>
#include <crow/proto/node.h>
#include <map>

#include <igris/event/delegate.h>

using namespace std::literals::chrono_literals;

namespace crow
{
    class spammer : public crow::node
    {
        struct record
        {
            std::chrono::time_point<std::chrono::system_clock> last_subscribe = {};
        };

        std::map<nodeaddr, record> targets = {};

        std::chrono::milliseconds timeout = 5000ms;
        uint8_t qos = 0;
        uint16_t ackquant = 50;

      public:
        void send(igris::buffer data)
        {
            auto time = std::chrono::system_clock::now();

            std::vector<std::map<nodeaddr, record>::iterator> to_delete;

            auto eit = targets.end();
            auto it = targets.begin();
            for (; it != eit; it++)
            {
                if (time - it->second.last_subscribe > timeout)
                {
                    to_delete.push_back(it);
                    continue;
                }

                node::send(it->first.nid, it->first.hostaddr(), data, qos,
                           ackquant);
            }

            for (auto it : to_delete)
            {
                targets.erase(it);
            }
        }

        void send_v(igris::buffer *data, size_t sz)
        {

            auto time = std::chrono::system_clock::now();

            std::vector<std::map<nodeaddr, record>::iterator> to_delete;

            auto eit = targets.end();
            auto it = targets.begin();
            for (; it != eit; it++)
            {
                if (time - it->second.last_subscribe > timeout)
                {
                    to_delete.push_back(it);
                    continue;
                }

                node::send_v(it->first.nid, it->first.hostaddr(), data, sz, qos,
                             ackquant);
            }

            for (auto it : to_delete)
            {
                targets.erase(it);
            }
        }

        void incoming_packet(crow::packet *pack) override
        {
            crow::node_packet_ptr npack(pack, this);
            auto time = std::chrono::system_clock::now();

            std::vector<uint8_t> addr(pack->addrptr(),
                                      pack->addrptr() + pack->addrsize());
            targets[nodeaddr{addr, (nodeid_t)npack.sid()}] = record{time};
        }

        int count_of_subscribers() { return targets.size(); }
    };

    class spam_subscriber : public crow::node
    {
        igris::delegate<void, igris::buffer> dlg = {};
        std::vector<uint8_t> addr = {};
        int nid = {};

      public:
        spam_subscriber(igris::delegate<void, igris::buffer> dlg) : dlg(dlg) {}

        void subscribe(nodeid_t nid, crow::hostaddr_view host, uint8_t qos = 2,
                       uint16_t ackquant = 200)
        {
            this->addr = std::vector<uint8_t>(
                (uint8_t *)host.data(), (uint8_t *)host.data() + host.size());
            this->nid = nid;
            node::send(nid, host, "", qos, ackquant);
        }

        void resubscribe(uint8_t qos = 2, uint16_t ackquant = 200)
        {
            node::send(nid, {addr.data(), addr.size()}, "", qos, ackquant);
        }

        void incoming_packet(crow::packet *pack) override
        {
            crow::node_packet_ptr npack(pack, this);
            dlg(npack.message());
        }
    };
}

#endif
