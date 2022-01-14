#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>

void crow::service_node::incoming_packet(crow::packet *pack)
{
    curpack = pack;
    auto &subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();

    auto message = igris::buffer{data.data() + 1 + reply_theme_length,
                                 data.size() - 1 - reply_theme_length};

    int anslen =
        dlg(message.data(), message.size(), *this);

    crow::release(pack);
}

void crow::service_node::reply(const char * data, size_t size) 
{
    auto &subheader = curpack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    reply_theme = igris::buffer(data.data() + 1, reply_theme_length);

    if (reply_theme != "__noanswer__")
        publish(pack->addr(), subheader.sid, reply_theme, {answer_buffer, anslen}, qos,
                ackquant);
}
