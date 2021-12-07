/** @file */

#ifndef CROW_BROCKER_CROWKER_H
#define CROW_BROCKER_CROWKER_H

#include <map>
#include <memory>
#include <string>

#include "theme.h"

#include "crow_client.h"
#include "tcp_client.h"
#include <crow/hostaddr.h>

using namespace crowker_implementation;

int64_t crowker_eval_timestamp();

namespace crow
{
    class crowker
    {
    public:
        // std::set<std::shared_ptr<client>> clients;
        std::map<std::string, theme> themes;
        bool brocker_info = false;
        bool log_publish = false;

    public:
        void set_info_mode(bool en) { brocker_info = en; }

        theme *get_theme(const std::string &name);

        void publish(const std::string &theme, const std::string &data);

        static crowker *instance()
        {
            static crowker *_instance = nullptr;
            if (_instance == nullptr)
                _instance = new crowker;

            return _instance;
        }

        void subscribe(const std::string &theme, client *cl);

        void tcp_subscribe(const std::string &theme,
                           nos::inet::tcp_socket *sock);

        void crow_subscribe(const crow::hostaddr_view &addr,
                            const std::string &theme, uint8_t qos,
                            uint16_t ackquant);

        void unlink_theme_client(crowker_implementation::theme *thm,
                                 crowker_implementation::client *sub);

        void erase_crow_client(const std::string &addr)
        {
            crowker_implementation::crow_client::allsubs.erase(addr);
        }

        void erase_tcp_client(const nos::inet::netaddr &addr)
        {
            crowker_implementation::tcp_client::allsubs.erase(addr);
        }

        std::vector<client *> clients()
        {
            std::vector<client *> ret;
            for (auto *client : crowker_implementation::crow_client::clients())
                ret.push_back(client);
            for (auto *client : crowker_implementation::tcp_client::clients())
                ret.push_back(client);
            return ret;
        }

    private:
        crowker() = default;
    };
} // namespace crow

#endif