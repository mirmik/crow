/**
@file
*/

#ifndef CROW_GATES_CHANNEL_GATE_H
#define CROW_GATES_CHANNEL_GATE_H

#include <crow/gateway.h>
#include <crow/packet.h>
#include <igris/buffer.h>

namespace crow
{
    class chardev_protocol;
    class chardev_driver;
    class chardev_gateway;

    class chardev_protocol
    {
      public:
        chardev_gateway *gate;
        chardev_driver *driver; // fast access

        dlist_head queue = DLIST_HEAD_INIT(queue);

        crow::packet *current;
        char *cursor;
        char *endptr;

      public:
        void send(crow::packet *pack);
        void newdata(const char *data, unsigned int size);

        void serve();
    };

    class chardev_driver
    {
      public:
        chardev_gateway *gate;

        virtual int room() = 0; // количество символов, которые устройство может
                                // принять. на передачу
        virtual void send(const char *data, unsigned int size) = 0;
        virtual void nonblock_tryread() = 0;
    };

    class chardev_gateway : public crow::gateway
    {
        chardev_driver *driver;
        chardev_protocol *protocol;

      public:
        chardev_gateway(chardev_driver *driver, chardev_protocol *protocol);

        void send(crow::packet *) override;
        void nblock_onestep() override;
        void finish() override;

      public: // driver callback
        void newdata(char c);
    };
}

#endif