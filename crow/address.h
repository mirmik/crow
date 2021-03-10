#ifndef CROW_ADDRESS_H
#define CROW_ADDRESS_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include <igris/dprint.h>
#include <crow/hostaddr.h>
#include <crow/hexer.h>

#include <vector>

namespace crow
{
	hostaddr address(const std::string& in);
	hostaddr address_warned(const std::string& in);
}

#endif