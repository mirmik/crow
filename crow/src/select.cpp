/** @file */

#include <crow/select.h>
#include <crow/tower.h>

#include <igris/math.h>
#include <nos/print.h>
#include <sys/select.h>

#include <vector>

static std::vector<int> fds;
bool crow::add_unselect_to_fds = false;

void crow::select_collect_fds()
{
    fds.clear();

    crow::gateway *gate;
    dlist_for_each_entry(gate, &crow::gateway_list, lnk)
    {
        auto vec = gate->get_fds();
        for (auto i : vec)
        {
            if (i >= 0)
                fds.push_back(i);
        }
    }

    if (add_unselect_to_fds)
        fds.push_back(crow::unselect_pipe[0]);
}

void crow::add_select_fd(int fd)
{
    fds.push_back(fd);
}

bool crow::select_need_to_update_fds()
{
    crow::gateway *gate;
    dlist_for_each_entry(gate, &crow::gateway_list, lnk)
    {
        if (gate->need_update_fds())
            return true;
    }
    return false;
}

void crow::select()
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    int nfds = 0;

    if (crow::select_need_to_update_fds())
        crow::select_collect_fds();

    for (int i : fds)
    {
        FD_SET(i, &read_fds);
        nfds = __MAX__(i, nfds);
    }

    int64_t timeout = crow::get_minimal_timeout();

    if (timeout < 0)
    {
        ::select(nfds + 1, &read_fds, NULL, NULL, NULL);
    }
    else
    {
        struct timeval timeout_struct =
        {
            (time_t)(timeout / 1000), (suseconds_t)((timeout % 1000) * 1000)
        };
        ::select(nfds + 1, &read_fds, NULL, NULL, &timeout_struct);
    }
}
