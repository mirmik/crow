/** @file */

#include "theme.h"
#include "subscriber.h"

#include <chrono>

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
        sub->publish(name, {data.data(), data.size()}, sub->thms[this].opts);
    }

    timestamp_publish = timestamp_activity = crowker_eval_timestamp();
}
