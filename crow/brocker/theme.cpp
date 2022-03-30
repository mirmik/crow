/** @file */

#include "theme.h"
#include "client.h"
#include <igris/dprint.h>
#include <igris/math.h>
#include <chrono>
#include <nos/print.h>


crowker_implementation::theme::theme(size_t queue_size) 
{
    last_messages.resize(queue_size);
}

int64_t crowker_eval_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

std::vector<std::shared_ptr<std::string>> crowker_implementation::theme::get_latest(uint32_t count) 
{
    std::vector<std::shared_ptr<std::string>> arr;
    uint32_t size = __MIN__(count, last_messages.size());
    for (size_t i = 0; i < size; ++i) 
    {
        arr.push_back(last_messages[i]);
    }
    return arr;
}

void crowker_implementation::theme::publish(const std::shared_ptr<std::string>& data)
{
    std::vector<client *> killme_list;

    {
        std::lock_guard<std::mutex> lock(mtx);
        last_messages.push(data);
    }

    for (auto *sub : subs)
    {
        crowker_implementation::options *opts = nullptr;
        if (sub->thms.count(this))
        {
            opts = sub->thms[this].opts;
        }

        if (crowker_eval_timestamp() - sub->timestamp_activity > 20000
            && sub->is_confirmed() == false)
        {
            killme_list.push_back(sub);
        }
        else
        {
            sub->publish(name, {data->data(), data->size()}, opts);
        }
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
