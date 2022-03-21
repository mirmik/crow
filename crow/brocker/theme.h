/** @file */

#ifndef CROWKER_THEME_H
#define CROWKER_THEME_H

#include <memory>
#include <set>
#include <string>
#include <mutex>
#include <deque>

#include <crow/defs.h>
#include <nos/print.h>
#include <igris/container/cyclic_buffer.h>

namespace crowker_implementation
{
    class client;

    class theme
    {
      public:
        std::string name={};
        std::set<client *> subs={};
        int64_t timestamp_publish = 0;
        int64_t timestamp_activity = 0;

        std::mutex mtx={};
        igris::cyclic_buffer<std::shared_ptr<std::string>> last_messages { CROW_DEFAULT_QUEUE_SIZE };

      public:
        theme(size_t queue_size);
        theme(const theme&) = delete;

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

        void publish(const std::shared_ptr<std::string>& data);
    };
}

#endif