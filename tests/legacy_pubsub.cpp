#include <crow/address.h>
#include <crow/brocker/crowker.h>
#include <crow/gateway.h>
#include <crow/pubsub/pubsub.h>
#include <crow/tower_cls.h>
#include <doctest/doctest.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace
{
    class capture_gateway : public crow::gateway
    {
    public:
        uint8_t type = 0;
        std::vector<uint8_t> data;

        void send(crow::packet *pack) override
        {
            type = pack->type();
            data.assign(pack->dataptr(), pack->dataptr() + pack->datasize());
            tower()->return_to_tower(pack, 0);
        }

        void finish() override {}
    };
}

TEST_CASE("legacy PubSub subscriber receives node-era broker publications")
{
    crow::Tower tower;
    capture_gateway gate;
    gate.bind(tower, 42);

    const std::string theme = "remoter_cmd";
    const auto address = crow::address(".42");

    crow::subheader_pubsub_control subscribe_header;
    subscribe_header.type = (uint8_t)crow::pubsub_type::SUBSCRIBE;
    subscribe_header.thmsz = theme.size();
    subscribe_header.qos = 0;
    subscribe_header.ackquant = 200;

    auto *subscribe = crow::allocate_packet<crow::header_v1>(
        address.size(), sizeof(subscribe_header) + theme.size());
    subscribe->set_type(CROW_PUBSUB_PROTOCOL);
    subscribe->set_quality(0);
    subscribe->set_ackquant(200);
    memcpy(subscribe->addrptr(), address.data(), address.size());
    memcpy(subscribe->dataptr(), &subscribe_header, sizeof(subscribe_header));
    memcpy(subscribe->dataptr() + sizeof(subscribe_header),
           theme.data(),
           theme.size());

    crow::pubsub_protocol.enable_crowker_subsystem();
    crow::pubsub_protocol.incoming(subscribe, tower);

    auto *broker = crow::crowker::instance();
    CHECK_EQ(broker->get_theme(theme)->count_clients(), 1);

    const std::string payload{"\x12\x34", 2};
    broker->publish(theme, std::make_shared<std::string>(payload));
    tower.onestep();

    REQUIRE_EQ(gate.type, CROW_PUBSUB_PROTOCOL);
    REQUIRE_EQ(gate.data.size(),
               sizeof(crow::subheader_pubsub_data) + theme.size() +
                   payload.size());

    auto *message_header =
        reinterpret_cast<crow::subheader_pubsub_data *>(gate.data.data());
    CHECK_EQ(message_header->type, uint8_t{2}); // legacy pubsub_type::MESSAGE
    CHECK_EQ(message_header->thmsz, theme.size());
    CHECK_EQ(message_header->datsz, payload.size());
    CHECK_EQ(std::string(message_header->theme().data(),
                         message_header->theme().size()),
             theme);
    CHECK_EQ(std::string(message_header->data().data(),
                         message_header->data().size()),
             payload);

    crowker_implementation::crow_client::allsubs.clear();
}
