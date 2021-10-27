#include "crow/iter.h"

std::vector<crow::gateway *> crow::gates()
{
    std::vector<crow::gateway *> ret;
    crow::gateway *ref;
    dlist_for_each_entry(ref, &crow::gateway_list, lnk) { ret.push_back(ref); }

    return ret;
}

std::vector<crow::node *> crow::nodes()
{
    std::vector<crow::node *> ret;

    crow::node *ref;
    dlist_for_each_entry(ref, &crow::nodes_list, lnk) { ret.push_back(ref); }

    return ret;
}
