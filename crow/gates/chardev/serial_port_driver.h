#ifndef CROW_SERIAL_PORT_DRIVER_H
#define CROW_SERIAL_PORT_DRIVER_H

#include <crow/gates/chardev_gateway.h>
#include <nos/io/serial_port.h>

namespace crow
{
    class serial_port_driver : public chardev_driver
    {
        nos::serial_port port;

      public:
        void start_send()
        {
            char buf[512];
            int len;

            while (1)
            {
                len = gateway->protocol->send_automate_getdata(buf, 512);
                if (len == 0)
                    break;

                port.write(buf, len);
            }

            gateway->packet_sended_handler();
        }

        void nonblock_tryread()
        {
            char c;
            ssize_t len = port.read((uint8_t *)&c, 1);
            if (len == 1)
                gateway->newdata(c);
        };
    };
}

#endif