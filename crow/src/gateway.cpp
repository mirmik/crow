#include <crow/gateway.h>
#include <crow/tower.h>

#include <igris/sync/syslock.h>

#include <nos/print.h>

igris::dlist<crow::gateway, &crow::gateway::lnk> crow::gateway_list;

int crow::gateway::bind(int id)
{
    gateway *g;

    system_lock();
    g = get_gateway(id);
    if (g)
        return -1;

    this->id = id;
    crow::gateway_list.add_last(*this);
    system_unlock();

    return 0;
}

crow::gateway *crow::get_gateway(int no)
{
    for (auto &g : crow::gateway_list)
    {
        if (g.id == no)
            return &g;
    }

    return nullptr;
}