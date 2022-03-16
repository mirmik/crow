/** @file */

#ifndef CROWKER_SUBSCRIBER_H
#define CROWKER_SUBSCRIBER_H

#include "options.h"
#include "theme.h"
#include <igris/buffer.h>
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
        std::string name={};
        crow::crowker *context=nullptr;
        std::unordered_map<theme *, themenote> thms={};
        int64_t timestamp_activity={};
        bool confirmation = false;

    public:
        // bool is_disconnected();
        virtual ~client();
        virtual void publish(const std::string &theme, const std::string &data,
                             options *opts) = 0;

        void set_confirmed(bool en) { confirmation = en; }

        bool is_confirmed() const { return confirmation; }
        void set_name(igris::buffer name)
        {
            this->name = std::string(name.data(), name.size());
        }

        void detach_from_themes()
        {
            for (auto &[key, value] : thms)
            {
                (void)value;
                key->unlink_client(this);
            }
        }
    };
}

#endif