#ifndef CROW_ADDRESS_H
#define CROW_ADDRESS_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include <crow/hexer.h>
#include <crow/hostaddr.h>

#include <vector>

namespace crow
{
    hostaddr crowker_address();
    hostaddr address(const std::string &in);
    hostaddr address_warned(const std::string &in);
}

#endif
