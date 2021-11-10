/** @file */

#ifndef CROW_WARN_H
#define CROW_WARN_H

#include <igris/buffer.h>

namespace crow
{
#if defined(MEMORY_ECONOMY)
    static inline void warn(igris::buffer) {}
#else
    void warn(igris::buffer msg);
#endif
}

#endif