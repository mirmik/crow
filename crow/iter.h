/** @file */

#ifndef CROW_ITER_H
#define CROW_ITER_H

#include <crow/gateway.h>
#include <crow/proto/node.h>
#include <crow/tower_cls.h>

#include <vector>

namespace crow
{
    std::vector<crow::gateway *> gates(Tower &tower);
    std::vector<crow::node *> nodes();
}

#endif
