/** @file */

#ifndef CROW_CLI_NODE
#define CROW_CLI_NODE

#include <crow/proto/node.h>
#include <igris/datastruct/argvc.h>
#include <igris/event/delegate.h>

namespace crow
{
    /// Нод для построения консольного интерфейса.
    /// В его обязанность входит получать сообщение, вызывать консольный
    /// обработчик в стиле igris/shell/rshell, после чего возвращать ответ
    /// отправителю.
    class rshell_cli_node : public crow::node
    {
        igris::delegate<int, char *, int, char *, int> handle;
        int answer_buffer_size = 96;

      public:
        rshell_cli_node(igris::delegate<int, char *, int, char *, int> handle)
            : handle(handle)
        {
        }

        void incoming_packet(crow_packet *pack) override;
        void undelivered_packet(crow_packet *pack) override
        {
            crow::release(pack);
        }
    };
}

#endif
