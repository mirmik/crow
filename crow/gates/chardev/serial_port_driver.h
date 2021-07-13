#ifndef CROW_SERIAL_PORT_DRIVER_H
#define CROW_SERIAL_PORT_DRIVER_H

#include <crow/gates/chardev_gateway.h>

namespace crow
{
    class serial_port_driver : public chardev_driver
    {
        int fd;

      public:
        int write(const char *data, unsigned int size)
        {
            ::write(fd, data, size);
        }

        void nonblock_tryread()
        {
            char c;
            ssize_t len = read(fd, (uint8_t *)&c, 1);
            if (len == 1)
                gateway->newdata(c);
        };
    }

#endif