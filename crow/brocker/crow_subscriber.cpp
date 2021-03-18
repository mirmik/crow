#include "crow_subscriber.h"
#include <crow/pubsub/pubsub.h>

std::map<std::string, crowker_implementation::crow_subscriber>
    crowker_implementation::crow_subscriber::allsubs;

void crowker_implementation::crow_subscriber::publish(
    const std::string &theme, const std::string &data,
    crowker_implementation::options *opts)
{
    crowker_implementation::crow_options *copts =
        static_cast<crowker_implementation::crow_options *>(opts);

    ::crow::publish({(uint8_t *)addr.data(), addr.size()}, theme.c_str(),
                    {data.data(), data.size()}, copts->qos, copts->ackquant,
                    MESSAGE);
}
