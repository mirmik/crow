#include <crow/nodes/service_node.h>
#include <crow/nodes/pubsub_defs.h>
#include <nos/print.h>

void crow::service_node::incoming_packet(crow_packet * pack) 
{
    char buf[48];

    auto& subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();

    int reply_theme_length = data.data()[0];
    auto reply_theme = igris::buffer(data.data()+1, reply_theme_length);
    auto message = igris::buffer { 
        data.data() + 1 + reply_theme_length, 
        data.size() - 1 - reply_theme_length };

    int anslen = dlg(message.data(), message.size(), buf, 48);
    auto answer = igris::buffer(buf, anslen);

    publish(
        pack->addr(),
        subheader.sid,
        reply_theme,
        answer,
        qos,
        ackquant
    );
}