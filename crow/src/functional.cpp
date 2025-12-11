#include <crow/address.h>
#include <crow/functional.h>

namespace crow
{
    std::list<std::unique_ptr<crow::subscriber_node>> subscribers;
}

crow::subscriber_node &crow::subscribe(Tower &tower,
                                       nos::buffer theme,
                                       std::function<void(nos::buffer)> func)
{
    auto addr = crow::crowker_address();
    auto node = std::make_unique<crow::subscriber_node>(addr, theme, func);
    node->bind(tower);
    node->install_keepalive(2000);
    subscribers.push_back(std::move(node));
    return *subscribers.back();
}

void crow::publish(Tower &tower, nos::buffer theme, nos::buffer data)
{
    auto addr = crow::crowker_address();
    crow::publisher_node node(addr, theme);
    node.bind(tower);
    node.publish(data);
}