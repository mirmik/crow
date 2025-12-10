#ifndef CROW_SERVICE_NODE_H
#define CROW_SERVICE_NODE_H

#include <crow/nodes/subscriber_node.h>

namespace crow
{
    // Chunked reply header format:
    // For single-packet reply (legacy, size <= chunk_size):
    //   [data...]
    // For chunked reply (size > chunk_size):
    //   [CHUNKED_REPLY_MARKER][chunk_id:2][flags:1][data...]
    //   flags: bit0 = has_more (1=more chunks follow, 0=last chunk)
    // Constants CHUNKED_REPLY_MARKER and CHUNK_FLAG_HAS_MORE are defined
    // in subscriber_node.h

    class service_node : public crow::abstract_subscriber_node
    {
        using delegate = igris::delegate<void, char *, int, service_node &>;
        delegate dlg = {};
        crow::packet *curpack = nullptr;
        size_t _chunk_size = 0; // 0 = no chunking (send as single packet)

    public:
        service_node() = default;
        service_node(const delegate &dlg) : dlg(dlg) {}
        service_node(const crow::hostaddr_view &addr,
                     const nos::buffer &theme,
                     const delegate &dlg)
            : abstract_subscriber_node(addr, theme), dlg(dlg)
        {
            set_theme(theme);
        }

        service_node(const service_node &) = delete;
        service_node &operator=(const service_node &) = delete;

        void set_handle(const delegate &dlg)
        {
            this->dlg = dlg;
        }

        void init(const delegate &dlg)
        {
            this->dlg = dlg;
        }

        void init(const crow::hostaddr_view &addr, const nos::buffer &theme)
        {
            abstract_subscriber_node::init_subscribe(addr, theme, 2, 50, 2, 50);
        }

        /// Set maximum chunk size for replies.
        /// If 0 (default), replies are sent as single packets (legacy mode).
        /// If > 0, large replies are split into chunks of this size.
        /// Note: actual payload per chunk = chunk_size - 4 (header overhead)
        void set_chunk_size(size_t size)
        {
            _chunk_size = size;
        }

        size_t chunk_size() const
        {
            return _chunk_size;
        }

        void reply(const char *data, size_t size);
        void reply(nos::buffer buf)
        {
            reply(buf.data(), buf.size());
        }

    private:
        void incoming_packet(crow::packet *) override;
        void reply_single(const char *data, size_t size);
        void reply_chunked(const char *data, size_t size);
    };
}

#endif