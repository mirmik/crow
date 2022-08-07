#ifndef CROW_FUNCTIONAL_H
#define CROW_FUNCTIONAL_H

#include <crow/nodes/subscriber_node.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace crow
{
    extern std::list<std::unique_ptr<crow::subscriber_node>> subscribers;

    crow::subscriber_node &subscribe(igris::buffer theme,
                                     std::function<void(igris::buffer)>);

    void publish(igris::buffer theme, igris::buffer data);
}

#endif
