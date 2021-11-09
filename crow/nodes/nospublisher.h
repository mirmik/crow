#ifndef CROW_NODES_NOSPUBLISHER_H
#define CROW_NODES_NOSPUBLISHER_H

#include <crow/nodes/publisher_node.h>
#include <nos/io/ostream.h>

namespace crow
{
    class nospublisher : public crow::publisher_node, public nos::ostream
    {
      public:
        nospublisher() = default;
        nospublisher(igris::buffer theme, crow::hostaddr_view crowker_addr,
                     int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO)
            : publisher_node(theme, crowker_addr, crowker_node)
        {
        }

        nospublisher(const nospublisher &) = default;

        int write(const void *ptr, size_t sz) override
        {
            crow::publisher_node::publish({ptr, sz});
            return sz;
        }
    };
}

#endif