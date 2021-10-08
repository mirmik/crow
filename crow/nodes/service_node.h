#ifndef CROW_SERVICE_NODE_H
#define CROW_SERVICE_NODE_H

#include <crow/nodes/subscriber_node.h>

namespace crow
{
    class service_node : public crow::abstract_subscriber_node
    {
        igris::delegate<int, char *, int, char *, int> dlg;
        int answer_buffer_size = 256;

      public:
        service_node(igris::delegate<int, char *, int, char *, int> dlg)
            : dlg(dlg)
        {
        }

        void set_handle(igris::delegate<int, char *, int, char *, int> dlg)
        {
            this->dlg = dlg;
        }

        service_node() {}

      private:
        void incoming_packet(crow_packet *) override;
    };
}

#endif