#ifndef CROW_PROTOCOL_H
#define CROW_PROTOCOL_H

#include <crow/packet.h>

namespace crow
{
    class Tower; // Forward declaration

    class protocol
    {
      public:
        virtual void incoming(crow::packet *pack, Tower &tower) = 0;
        virtual void undelivered(crow::packet *pack, Tower &tower) = 0;
        virtual ~protocol() = default;
    };
}

#endif