#ifndef CROW_QOSBYTE_H
#define CROW_QOSBYTE_H

#include <cmath>

namespace crow
{
    static constexpr int quant_const = 10;
    class qosbyte
    {
    public:
        uint8_t _qos : 2;
        uint8_t _quant_code : 6;

    private:
        uint8_t quant_to_code(uint16_t quant)
        {
            return std::round((float)(quant) / quant_const);
        }

    public:
        uint16_t quant() { return _quant_code * quant_const; }
        uint8_t quality() { return _qos; }

        void set_quant(uint16_t quant) { _quant_code = quant_to_code(quant); }
        void set_quality(uint8_t quality) { _qos = quality; }
    };
    static_assert(sizeof(qosbyte) == 1, "qosbyte wrong size");
    static_assert(std::is_trivial<qosbyte>::value == true,
                  "qosbyte is not trivial");
}

#endif