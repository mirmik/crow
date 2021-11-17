/** @file */

#include "theme.h"
#include "client.h"
#include <igris/dprint.h>
#include <chrono>
#include <nos/print.h>

int64_t crowker_eval_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

void crowker_implementation::theme::publish(const std::string &data)
{
    std::vector<client *> killme_list;

    for (auto *sub : subs)
    {
        crowker_implementation::options *opts = nullptr;
        if (sub->thms.count(this))
            opts = sub->thms[this].opts;

        //if (crowker_eval_timestamp() - sub->timestamp_activity > 20000)
        //{
        //    killme_list.push_back(sub);
        //}
        //else
        //{
            sub->publish(name, {data.data(), data.size()}, opts);
        //}
    }

    for (auto * sub : killme_list)
    {
        subs.erase(sub);
        nos::println("theme drop client");

        if (sub->thms.size() == 0)
            delete sub;
    }

    timestamp_publish = timestamp_activity = crowker_eval_timestamp();
}
