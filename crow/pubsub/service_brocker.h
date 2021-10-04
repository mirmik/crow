#ifndef CROW_PUBSUB_SERVICE_BROCKER_H
#define CROW_PUBSUB_SERVICE_BROCKER_H

#include <crow/proto/node.h>
#include <crow/pubsub/service_defs.h>

#include <unordered_map>

namespace crow
{
    class service_record
    {
        crow::hostaddr host;
        int sid;

      public:
        service_record(crow::hostaddr host, int sid) : host(host), sid(sid) {}
    };

    class service_brocker : public crow::node
    {
        std::unordered_map<std::string, service_record> records;

      private:
        bool has_service(igris::buffer theme)
        {
            return (bool)records.count(std::string(theme.data(), theme.size()));
        }

        void create_service(igris::buffer theme, igris::buffer addr, int sid)
        {
            auto strtheme = std::string(theme.data(), theme.size());
            auto host = crow::hostaddr(addr);

            records.emplace(
                std::make_pair(strtheme, service_record(std::move(host), sid)));
        }

        void declare_service_handle(crow_packet *pack)
        {
            service_packet_subheader_declare_service *header =
                (service_packet_subheader_declare_service *)node_data(pack)
                    .data();

            int header_size = sizeof(service_packet_subheader_declare_service);
            igris::buffer theme = {(char *)header + header_size,
                                   (size_t)(pack->datasize() - header_size)};

            if (!has_service(theme))
            {
                node_subheader *nsub = (crow::node_subheader *)pack->dataptr();
                create_service(theme, pack->addr(), nsub->sid);
            }

            crow::release(pack);
        }

        void request_handle(crow_packet *pack)
        {
            service_packet_subheader_request *header =
                (service_packet_subheader_request *)node_data(pack).data();
        }

        void reply_handle(crow_packet *pack) {}

        void incoming_packet(crow_packet *pack) override
        {
            auto data = crow::node_data(pack);

            service_packet_subheader_base *service_header =
                (service_packet_subheader_base *)data.data();
            auto subtype = service_header->subtype;

            switch (subtype)
            {
            case service_packet_subheader_subtype::DeclareService:
                declare_service_handle(pack);
                break;

            case service_packet_subheader_subtype::Request:
                request_handle(pack);
                break;

            case service_packet_subheader_subtype::Reply:
                reply_handle(pack);
                break;
            }
        }
    };
}

#endif