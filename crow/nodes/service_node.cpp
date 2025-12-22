#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>
#include <cstring>

void crow::service_node::incoming_packet(crow::packet *pack)
{
    curpack = pack;
    auto &subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    auto message = nos::buffer{data.data() + 1 + reply_theme_length,
                               data.size() - 1 - reply_theme_length};
    dlg(message.data(), message.size(), *this);

    // release after reply
    _tower->release(pack);
}

// Static buffer for assembling chunked reply messages.
// Size: max reply ~4KB + headers. Single-threaded use assumed.
static char service_reply_buffer[4096 + 64];

void crow::service_node::reply(const char *answ, size_t size)
{
    auto &subheader = curpack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    auto reply_theme = nos::buffer(data.data() + 1, reply_theme_length);

    if (reply_theme == "__noanswer__")
        return;

    // Build pubsub header
    crow::publish_subheader sh;
    sh.type = PubSubTypes::Publish;
    sh.thmsize = reply_theme.size();
    sh.datsize = size;

    // If chunking is enabled and message is large, use node-level chunking
    if (_chunk_size > 0)
    {
        // Calculate total message size
        size_t pubsub_header_size = sizeof(sh) - sizeof(node_subheader);
        size_t total_size = pubsub_header_size + reply_theme.size() + size;

        // Check buffer capacity
        if (total_size > sizeof(service_reply_buffer))
        {
            crow::warn("service_node::reply: message too large, truncating");
            size = sizeof(service_reply_buffer) - pubsub_header_size -
                   reply_theme.size();
            total_size = sizeof(service_reply_buffer);
            sh.datsize = size;
        }

        // Assemble message into static buffer
        char *ptr = service_reply_buffer;

        // Add pubsub header (excluding node_subheader part)
        size_t header_part = sizeof(sh) - sizeof(node_subheader);
        memcpy(ptr, reinterpret_cast<char *>(&sh) + sizeof(node_subheader),
               header_part);
        ptr += header_part;

        // Add theme
        memcpy(ptr, reply_theme.data(), reply_theme.size());
        ptr += reply_theme.size();

        // Add data
        memcpy(ptr, answ, size);

        // Send using node-level chunking
        send_chunked(subheader.sid, curpack->addr(),
                     {service_reply_buffer, total_size}, qos, ackquant);
    }
    else
    {
        // No chunking - use standard publish
        publish(curpack->addr(), subheader.sid, reply_theme, {answ, size}, qos,
                ackquant);
    }
}
