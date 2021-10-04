#ifndef CROW_PUBSUB_SERVICE_DEFS_H
#define CROW_PUBSUB_SERVICE_DEFS_H

namespace crow
{
    enum class service_packet_subheader_subtype : uint8_t
    {
        DeclareService = 0,
        Request = 1,
        Reply = 2
    };

    struct service_packet_subheader_base
    {
        service_packet_subheader_subtype subtype;
    };

    struct service_packet_subheader_declare_service
    {
        service_packet_subheader_base base = {
            service_packet_subheader_subtype::DeclareService};
    };

    struct service_packet_subheader_request
    {
        service_packet_subheader_base base = {
            service_packet_subheader_subtype::Request};
        uint8_t theme_size;
        uint16_t data_size;
    };

    struct service_packet_subheader_reply
    {
        service_packet_subheader_base base = {
            service_packet_subheader_subtype::Reply};
    };
}

#endif