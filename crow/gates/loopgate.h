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

        void send(crow_packet *pack) override
        {
            crow_packet *copypack = crow::allocate_packet(
                crow_packet_addrsize(pack) + crow_packet_datasize(pack));

            memcpy(copypack, pack,
                   sizeof(crow_packet) + crow_packet_addrsize(pack) +
                       crow_packet_datasize(pack));

            crow_packet_initialization(copypack, this);
            crow_packet_revert_gate(copypack, id);

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
