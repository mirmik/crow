/** @file */

#ifndef CROWKER_SUBSCRIBER_H
#define CROWKER_SUBSCRIBER_H

#include "options.h"
#include "theme.h"
//#include <crow/brocker/service.h>

#include <unordered_map>

namespace crow
{
    class crowker;
}

namespace crowker_implementation
{
    struct themenote
    {
        options *opts = nullptr;
        ~themenote() { delete opts; }
    };

    class client
    {
      public:
        crow::crowker *context;
        std::unordered_map<theme *, themenote> thms;

      public:
        // bool is_disconnected();
        virtual ~client();
        virtual void publish(const std::string &theme, const std::string &data,
                             options *opts) = 0;
    };
}

#endif