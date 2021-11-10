#ifndef CROW_QOSBYTE_H
#define CROW_QOSBYTE_H

#include <cmath>

namespace crow
{
    class qosbyte
    {
        uint8_t qos : 2;
        uint8_t quant_code : 6;

    public:
        qosbyte(uint8_t qos, uint16_t ackquant)
            : quant_code(std::round((float)ackquant / 15)), qos(qos)
        {
        }
    };
    static_assert(sizeof(qosbyte) == 1, "qosbyte wrong size");
}

#endif