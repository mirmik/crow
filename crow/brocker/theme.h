/** @file */

#ifndef CROWKER_THEME_H
#define CROWKER_THEME_H

#include <memory>
#include <set>
#include <string>

#include <nos/print.h>

namespace crowker_implementation
{
    class client;

    class theme
    {
      public:
        std::string name;
        std::set<client *> subs;
        int64_t timestamp_publish;
        int64_t timestamp_activity;

      public:
        size_t count_clients() { return subs.size(); }

        bool link_client(client *sub)
        {
            if (subs.count(sub) == 0)
            {
                subs.insert(sub);
                return true;
            }

            return false;
        }

        bool has_client(client *sub) { return subs.count(sub); }

        void unlink_client(client *sub)
        {
            nos::println("unlink");
            subs.erase(sub);
        }

        void publish(const std::string &data);
    };
}

#endif