/** @file */

#ifndef CROW_PRINT_H
#define CROW_PRINT_H

#include <crow/packet.h>
#include <string>

namespace crow
{
    void diagnostic(const char *notation,
                    crow::packet *pack,
                    const std::string &label,
                    uint16_t dbg_size);
}

#endif
