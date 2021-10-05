#ifndef CROW_PUBSUB_SERVICE_DEFS_H
#define CROW_PUBSUB_SERVICE_DEFS_H

#include <crow/packet.h>
#include <crow/proto/node.h>

namespace crow
{
    enum class service_packet_subheader_subtype : uint8_t
    {
        DeclareService = 0,
        ProtocolError = 1,

        Request = 2,
        Reply = 3,

        ServiceRequest = 4,
        ServiceReply = 5,
    };

    class service_packet_subheader_base : public crow::node_subheader
    {
      public:
        service_packet_subheader_subtype subtype;
    } __attribute__((packed));

    class service_packet_subheader_declare_service
    {
      public:
        service_packet_subheader_base base;

        igris::buffer theme()
        {
            return {
                (char *)(this + 1),
            };
        }
    } __attribute__((packed));

    class service_packet_subheader_request
        : public service_packet_subheader_base
    {
      public:
        service_packet_subheader_base base;
        uint8_t theme_size;
        int16_t request_number;

        igris::buffer theme() { return {(char *)(this + 1), theme_size}; }

        igris::buffer message(crow::packet &pack)
        {
            return {(char *)(this + 1) + theme_size,
                    (size_t)(pack.datasize() - theme_size)};
        }
    } __attribute__((packed));

    class service_packet_subheader_service_request
    {
      public:
        service_packet_subheader_base base;
        int16_t service_request_number;
    } __attribute__((packed));

    class service_packet_subheader_reply
    {
      public:
        service_packet_subheader_base base;
        int16_t service_request_number;
    } __attribute__((packed));

    class service_packet_subheader_service_reply
    {
      public:
        service_packet_subheader_base base;
        int16_t request_number;
    } __attribute__((packed));

    static inline const char *
    subtype_to_cstr(service_packet_subheader_subtype type)
    {
        switch (type)
        {

        case service_packet_subheader_subtype::DeclareService:
            return "DeclareService";

        case service_packet_subheader_subtype::Request:
            return "Request";

        case service_packet_subheader_subtype::Reply:
            return "Reply";

        case service_packet_subheader_subtype::ServiceRequest:
            return "ServiceRequest";

        case service_packet_subheader_subtype::ServiceReply:
            return "ServiceReply";

        case service_packet_subheader_subtype::ProtocolError:
            return "ProtocolError";

        default:
            return "UnresolvedPacketType";
        }
    }
}

#endif