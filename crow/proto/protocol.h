#ifndef CROW_PROTOCOL_H
#define CROW_PROTOCOL_H

namespace crow
{
    class protocol
    {
      public:
        virtual void incoming(crow::packet *pack) = 0;
        virtual void undelivered(crow::packet *pack) = 0;
    };
}

#endif