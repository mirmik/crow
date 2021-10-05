#ifndef CROW_PUBSUB_SERVICE_BROCKER_H
#define CROW_PUBSUB_SERVICE_BROCKER_H

#include <crow/proto/node.h>
#include <crow/pubsub/service_defs.h>

#include <unordered_map>

namespace crow
{
    class service_record
    {
      public:
        crow::hostaddr host;
        int node;

      public:
        service_record(crow::hostaddr host, int sid) : host(host), node(sid) {}
    };

    class service_brocker : public crow::node
    {
        std::unordered_map<std::string, service_record> records;
        bool debug_mode = false;

      private:
        void set_debug_mode(bool en)
        {
            debug_mode = en;

            if (debug_mode)
                nos::println("service_brocker : debug mode enabled");
        }

        bool has_service(igris::buffer theme)
        {
            return (bool)records.count(std::string(theme.data(), theme.size()));
        }

        service_record *find_record(igris::buffer theme)
        {
            std::string strtheme = {theme.data(), theme.size()};
            auto it = records.find(strtheme);
            return it == records.end() ? nullptr : &(*it).second;
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
            auto &header =
                pack->subheader<service_packet_subheader_declare_service>();
            igris::buffer theme = header.theme();

            if (!has_service(theme))
            {
                node_subheader *nsub = (crow::node_subheader *)pack->dataptr();
                create_service(theme, pack->addr(), nsub->sid);
            }

            crow::release(pack);
        }

        void send_message_to_service(service_record &record,
                                     service_packet_subheader_request &header,
                                     crow::packet *pack)
        {
            const auto &host = record.host;
            const auto &node = record.node;
            auto data = header.message(*pack);

            send(node, host, data);
        }

        void request_handle(crow_packet *pack)
        {
            auto &header = pack->subheader<service_packet_subheader_request>();

            auto theme = header.theme();
            auto *record = find_record(theme);

            if (record == nullptr)
            {
                if (debug_mode)
                {
                    nos::println("service_brocker: service {} not found",
                                 theme);
                }

                crow::release(pack);
                return;
            }

            send_message_to_service(*record, header, pack);
            crow::release(pack);
        }

        void service_reply_handle(crow_packet *pack)
        {
            nos::println("TODO : SERVICE REPLY HANDLE");
            (void)pack;
        }

        void send_protocol_error_answer(crow_packet *pack)
        {
            nos::println("TODO : SEND PROTOCOL ERROR");
            (void)pack;
        }

        void unresolved_packet_handle(crow_packet *pack)
        {
            send_protocol_error_answer(pack);
            crow::release(pack);
        }

        void protocol_error_handle(crow_packet *pack)
        {
            if (debug_mode)
                nos::println("service_brocker: error message");

            crow::release(pack);
        }

        void incoming_packet(crow_packet *pack) override
        {
            auto &header = pack->subheader<service_packet_subheader_base>();
            auto subtype = header.subtype;

            if (debug_mode)
                nos::println("service_brocker : newpacket. subtype:{}",
                             subtype_to_cstr(subtype));

            switch (subtype)
            {
            case service_packet_subheader_subtype::DeclareService:
                declare_service_handle(pack);
                break;

            case service_packet_subheader_subtype::Request:
                request_handle(pack);
                break;

            case service_packet_subheader_subtype::Reply:
                service_reply_handle(pack);
                break;

            case service_packet_subheader_subtype::ProtocolError:
                protocol_error_handle(pack);
                break;

            default:
                unresolved_packet_handle(pack);
                break;
            }
        }
    };
}

#endif