/** @file */

#ifndef CROW_WARN_H
#define CROW_WARN_H

#include <string_view>

namespace crow
{
    void warn(std::string_view msg);
}

#endif