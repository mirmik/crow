#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>

void crow::service_node::incoming_packet(crow_packet *pack)
{
    static char answer [16];
    memset(answer, 0, 16);
    answer_buffer_size = 15;
    //char *answer = (char *)malloc(answer_buffer_size);
    //memset(answer, 0, answer_buffer_size);

    auto &subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();

    int reply_theme_length = data.data()[0];
    auto reply_theme = igris::buffer(data.data() + 1, reply_theme_length);
    auto message = igris::buffer{data.data() + 1 + reply_theme_length,
                                 data.size() - 1 - reply_theme_length};

    int anslen =
        dlg(message.data(), message.size(), answer, answer_buffer_size);

    publish(pack->addr(), subheader.sid, reply_theme, {answer, anslen}, qos,
            ackquant);

    crow::release(pack);
}