#ifndef CROW_SERVICE_NODE_H
#define CROW_SERVICE_NODE_H

#include <crow/nodes/subscriber_node.h>

namespace crow
{
    class service_node : public crow::abstract_subscriber_node
    {
        using delegate = igris::delegate<void, char *, int, service_node&>;
        delegate dlg;
        crow::packet * curpack;

    public:
        service_node() = default;
        service_node(const delegate& dlg) : dlg(dlg) {}
        
        void set_handle(const delegate& dlg) 
        {
            this->dlg = dlg;
        }

        void init(const delegate& dlg)
        {
            this->dlg = dlg;
        }

        void reply(const char * data, size_t size);

    private:
        void incoming_packet(crow::packet *) override;
    };
}

#endif