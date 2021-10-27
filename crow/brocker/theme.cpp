/** @file */

#include "theme.h"
#include "client.h"

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
    for (auto *sub : subs)
    {
        crowker_implementation::options *opts = nullptr;
        if (sub->thms.count(this))
            opts = sub->thms[this].opts;
        sub->publish(name, {data.data(), data.size()}, opts);
    }
    timestamp_publish = timestamp_activity = crowker_eval_timestamp();
}
