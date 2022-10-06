/** @file */

#include "crow_client.h"
#include <crow/pubsub/pubsub.h>

std::map<std::string, crowker_implementation::crow_client>
    crowker_implementation::crow_client::allsubs;

void crowker_implementation::crow_client::publish(
    const std::string &theme,
    const std::string &data,
    crowker_implementation::options *opts)
{
    (void)theme;
    (void)data;
    (void)opts;

#ifdef CROW_PUBSUB_PROTOCOL_SUPPORTED
    crowker_implementation::crow_options *copts =
        static_cast<crowker_implementation::crow_options *>(opts);
    ::crow::publish({(uint8_t *)addr.data(), addr.size()}, theme.c_str(),
                    {data.data(), data.size()}, copts->qos, copts->ackquant,
                    (uint8_t)crow::pubsub_type::MESSAGE);
#endif
}
