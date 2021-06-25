/** @file */

#include "tcp_subscriber.h"

std::map<nos::inet::netaddr, crowker_implementation::tcp_subscriber>
    crowker_implementation::tcp_subscriber::allsubs;
void crowker_implementation::tcp_subscriber::publish(const std::string &theme,
                                                     const std::string &data,
                                                     options *opts)
{
    assert(opts == nullptr);
    std::string str = nos::format("p{:f02>}{}{:f06>}{}", theme.size(), theme,
                                  data.size(), data);
    sock->write(str.data(), str.size());
}