/** @file */

#ifndef CROW_GATES_LOOPGATE_H
#define CROW_GATES_LOOPGATE_H

#include <crow/gateway.h>
#include <crow/tower.h>

namespace crow
{
    class loopgate : public gateway
    {
      public:
        loopgate() {}

        void send(crow::packet *pack) override
        {
            crow::packet *copypack =
                crow::allocate_packet(pack->addrsize() + pack->datasize());

            memcpy(copypack, pack,
                   sizeof(crow::packet) + pack->addrsize() + pack->datasize());

            crow::packet_initialization(copypack, this);
            copypack->revert_gate(id);

            crow::return_to_tower(pack, 0);
            crow::nocontrol_travel(copypack, false);
        }
        void nblock_onestep() {}

        void finish() override {}
    };

    int create_loopgate(uint8_t id)
    {
        auto *gate = new loopgate;
        gate->bind(id);

        return 0;
    }
}

#endif
