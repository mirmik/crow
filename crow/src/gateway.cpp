#include <crow/gateway.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>

#include <igris/sync/syslock.h>

// C-style gateway binding - uses default_tower()
int crow_gateway_bind(struct crow_gateway *gate, int id)
{
    return crow::default_tower().bind_c_gateway(gate, id);
}

struct crow_gateway *crow_get_gateway(int no)
{
    return crow::default_tower().get_c_gateway(no);
}

// C++ gateway binding - uses default_tower()
int crow::gateway::bind(int id)
{
    return bind(crow::default_tower(), id);
}

// C++ gateway binding with explicit tower
int crow::gateway::bind(Tower &tower, int id)
{
    _tower = &tower;
    return tower.bind_gateway(this, id);
}

crow::gateway *crow::get_gateway(int no)
{
    return crow::default_tower().get_gateway(no);
}

// Compatibility: provide global gateway_list references
// These are deprecated - new code should use default_tower().gateway_list()
namespace crow
{
    dlist_head &gateway_list = default_tower().gateway_list();
}

// For C code compatibility
struct dlist_head &crow_gateway_list = crow::default_tower().c_gateway_list();
