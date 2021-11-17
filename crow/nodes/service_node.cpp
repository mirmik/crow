#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>

void crow::service_node::incoming_packet(crow::packet *pack)
{
    if (dynamic_answer_buffer)
    {
        answer_buffer = (char *)malloc(answer_buffer_size);
        memset(answer_buffer, 0, answer_buffer_size);
    }

    auto &subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();

    int reply_theme_length = data.data()[0];
    auto reply_theme = igris::buffer(data.data() + 1, reply_theme_length);
    auto message = igris::buffer{data.data() + 1 + reply_theme_length,
                                 data.size() - 1 - reply_theme_length};

    int anslen =
        dlg(message.data(), message.size(), answer_buffer, answer_buffer_size);

    if (reply_theme != "__noanswer__")
        publish(pack->addr(), subheader.sid, reply_theme, {answer_buffer, anslen}, qos,
                ackquant);

    if (dynamic_answer_buffer)
    {
        free(answer_buffer);
    }

    crow::release(pack);
}