#include <crow/gateway.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>

#include <igris/sync/syslock.h>

// C++ gateway binding with explicit tower
int crow::gateway::bind(Tower &tower, int id)
{
    _tower = &tower;
    return tower.bind_gateway(this, id);
}
