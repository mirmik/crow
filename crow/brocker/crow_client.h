/** @file */

#ifndef CROW_BROCKER_CROW_SUBSCRIBER_H
#define CROW_BROCKER_CROW_SUBSCRIBER_H

#include <map>
#include <string>

#include "options.h"
#include "client.h"

namespace crowker_implementation
{
    class crow_client : public client
    {
      public:
        std::string addr;

        static std::map<std::string, crow_client> allsubs;
        static crow_client *get(const std::string &addr)
        {
            return &allsubs[addr];
        }

        void publish(const std::string &theme, const std::string &data,
                     options *opts) override;
    };
} // namespace crowker_implementation

#endif