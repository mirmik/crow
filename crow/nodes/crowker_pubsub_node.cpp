#include <crow/nodes/crowker_pubsub_node.h>
#include <crow/nodes/pubsub_defs.h>

#include <nos/fprint.h>

void crow::crowker_pubsub_node::incoming_packet(crow::packet *pack)
{
    auto &s = pack->subheader<pubsub_subheader>();

    switch (s.type)
    {
        case PubSubTypes::Publish:
        {
            auto &sh = pack->subheader<publish_subheader>();
            auto message = sh.message();
            api->publish_to_theme(sh.theme(), std::make_shared<std::string>(
                message.data(), message.size()));
        };
        break;

        case PubSubTypes::Subscribe:
        {
            auto &sh = pack->subheader<subscribe_subheader>();
            api->subscribe_on_theme(pack->addr(), sh.sid, sh.theme(), sh.rqos,
                                    sh.rackquant);
        };
        break;

        case PubSubTypes::Request:
        {
            auto &sh = pack->subheader<request_subheader>();
            api->subscribe_on_theme(pack->addr(), sh.sid, sh.reply_theme(), sh.rqos,
                                    sh.rackquant);

            uint8_t len = sh.reply_theme().size();
            auto msg = std::make_shared<std::string>(nos::format("{}{}{}", 
                igris::buffer(&len, 1),
                sh.reply_theme(), 
                sh.message()));
            api->publish_to_theme(sh.theme(), msg);
        };
        break;

        default:
            break;
    }

    crow::release(pack);
}

void crow::crowker_pubsub_node::undelivered_packet(crow::packet * pack) 
{
    auto node = crow::node_protocol_cls::rid(pack);
    api->undelivered_packet(pack->addr(), node);
    crow::release(pack);
}
