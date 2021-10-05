#ifndef CROW_BROCKER_BROCKER_H
#define CROW_BROCKER_BROCKER_H

#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <igris/buffer.h>

namespace crow
{
    class brocker_theme;
    class brocker_service;

    class brocker_client
    {
        int refs;

        std::set<brocker_theme *> my_themes;
        std::set<brocker_service *> my_services;

      public:
        virtual void send_reply_to_service_client(igris::buffer data) = 0;
        virtual void send_request_to_service(igris::buffer data) = 0;
        virtual void send_error_unresolved_service() = 0;

        virtual void send_pubsub_message(brocker_theme *theme,
                                         igris::buffer data) = 0;
        virtual ~brocker_client();
    };

    class brocker_service_request
    {
        int client_to_brocker_id;
        int brocker_to_service_id;
        std::shared_ptr<brocker_client> sender;
    };

    class brocker_theme
    {
        std::list<std::shared_ptr<crow::brocker_client>> subscribers;
    };

    class brocker_service
    {
        int request_id_counter = 0;
        std::shared_ptr<crow::brocker_client> service_owner;
        std::list<std::shared_ptr<crow::brocker_service_request>> requestes;
    };

    class brocker
    {
        std::unordered_map<int64_t, std::shared_ptr<crow::brocker_client>>
            clients;
        std::unordered_map<std::string, crow::brocker_theme> themes;
        std::unordered_map<std::string, crow::brocker_service> services;

        int service_request(std::shared_ptr<brocker_service_request> record,
                            igris::buffer theme, igris::buffer data);
    };
}

#endif