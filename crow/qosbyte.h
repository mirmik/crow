#ifndef CROW_QOSBYTE_H
#define CROW_QOSBYTE_H

#include <cmath>

namespace crow
{
    class qosbyte
    {
        uint8_t _qos : 2;
        uint8_t _quant_code : 6;

        constexpr int quant_const = 10;

    public:
        qosbyte(uint8_t qos, uint16_t ackquant)
            : _quant_code(std::round((float)(ackquant) / quant_const)), qos(qos)
        {
        }

        uint16_t quant() { return _quant_code * quant_const; }
        uint8_t quality() { return _qos; }
    };
    static_assert(sizeof(qosbyte) == 1, "qosbyte wrong size");
}

#endif