#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>
#include <cstring>
#include <vector>

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

        // Assemble complete message
        std::vector<char> message_buf;
        message_buf.reserve(total_size);

        // Add pubsub header (excluding node_subheader part)
        message_buf.insert(message_buf.end(),
                           reinterpret_cast<char *>(&sh) + sizeof(node_subheader),
                           reinterpret_cast<char *>(&sh) + sizeof(sh));

        // Add theme
        message_buf.insert(message_buf.end(), reply_theme.data(),
                           reply_theme.data() + reply_theme.size());

        // Add data
        message_buf.insert(message_buf.end(), answ, answ + size);

        // Send using node-level chunking
        send_chunked(subheader.sid, curpack->addr(),
                     {message_buf.data(), message_buf.size()}, qos, ackquant);
    }
    else
    {
        // No chunking - use standard publish
        publish(curpack->addr(), subheader.sid, reply_theme, {answ, size}, qos,
                ackquant);
    }
}
