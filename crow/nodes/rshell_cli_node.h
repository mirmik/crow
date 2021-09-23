/** @file */

#ifndef CROW_CLI_NODE_RSHELL
#define CROW_CLI_NODE_RSHELL

#include <crow/proto/node.h>
#include <igris/datastruct/argvc.h>
#include <igris/event/delegate.h>

#include <igris/shell/rshell_executor.h>

namespace crow
{
    /// Нод для построения консольного интерфейса.
    /// В его обязанность входит получать сообщение, вызывать консольный
    /// обработчик в стиле igris/shell/rshell, после чего возвращать ответ
    /// отправителю.
    class rshell_cli_node_base : public crow::node
    {
        int answer_buffer_size = 96;

      public:
        void incoming_packet(crow_packet *pack) override;
        void undelivered_packet(crow_packet *pack) override
        {
            crow::release(pack);
        }

        virtual int handle(char *, int, char *, int) = 0;
    };

    class rshell_cli_node_delegate : public rshell_cli_node_base
    {
        igris::delegate<void, char *, int, char *, int> dlg;

      public:
        rshell_cli_node_delegate(
            igris::delegate<void, char *, int, char *, int> handle)
            : dlg(handle)
        {
        }

        int handle(char *a, int b, char *c, int d) override
        {
            dlg(a, b, c, d);
            int anslen = strlen(c);
            return anslen;
        }
    };

    class rshell_cli_node : public rshell_cli_node_base
    {
        igris::rshell_executor *_executor;

      public:
        rshell_cli_node() {}
        rshell_cli_node(igris::rshell_executor *executor) : _executor(executor)
        {
        }

        void set_executor(igris::rshell_executor *executor)
        {
            _executor = executor;
        }

        int handle(char *a, int b, char *c, int d) override
        {
            _executor->execute(a, b, c, d);
            int anslen = strlen(c);
            return anslen;
        }
    };
}

#endif