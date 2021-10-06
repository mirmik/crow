/** @file */

#include "pubsub.h"
#include "subscriber.h"
#include <crow/brocker/crowker.h>

#include <igris/util/bug.h>

void incoming_crowker_handler(struct crow_packet *pack)
{

    crow::subheader_pubsub & shps = pack->subheader<crow::subheader_pubsub>();

    switch (shps.type)
    {
        case PUBLISH:
        {
            auto& shps_d = pack->subheader<crow::subheader_pubsub_data>();
            auto theme = shps_d.theme().to_string();
            auto data = shps_d.data().to_string();
            crow::crowker::instance()->publish(theme, data);
        }
        break;

        case SUBSCRIBE:
        {
            auto& shps_c = pack->subheader<crow::subheader_pubsub_control>();
            std::string theme(crow_packet_dataptr(pack) + sizeof(crow::subheader_pubsub) +
                              sizeof(crow::subheader_pubsub_control),
                              shps.thmsz);

            crow::crowker::instance()->crow_subscribe(
            {crow_packet_addrptr(pack), crow_packet_addrsize(pack)}, theme, shps_c.qos,
            shps_c.ackquant);
        }
        break;

        case SERVICE_ANSWER:
        {
            BUG();
        }
        break;

        default:
        {
            printf("unresolved pubsub frame type %d", (uint8_t)shps.type);
        }
        break;

        //case MESSAGE:
        //{
        //    BUG();

        /*    crow::subscriber *sub;
            igris::buffer theme = crow::pubsub::get_theme(pack);

            dlist_for_each_entry(sub, &crow::pubsub_protocol.subscribers, lnk)
            {
                if (theme == sub->theme)
                {
                    sub->newpack_handler(pack);
                    return;
                }
            }*/
        //}
    }

    crow::release(pack);
}

void undelivered_crowker_handler(struct crow_packet *pack)
{
    auto& shps = pack->subheader<crow::subheader_pubsub>();

    if (shps.type == PUBLISH)
    {
        crow::crowker::instance()->erase_crow_client(
            std::string((char *)crow_packet_addrptr(pack), pack->header.alen));
    }

    crow::release(pack);
}

void crow::pubsub_protocol_cls::enable_crowker_subsystem()
{
    instance().incoming_handler = incoming_crowker_handler;
    instance().undelivered_handler = undelivered_crowker_handler;
}
