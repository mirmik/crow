/** @file */

#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#include <crow/defs.h>
#include <crow/gateway.h>
#include <crow/packet_ptr.h>
#include <igris/event/delegate.h>
#include <nos/buffer.h>
#include <string>

// Forward declaration of Tower class
namespace crow
{
    class Tower;
}

// Tower-specific functions
namespace crow
{
    // Spin/thread functions (require Tower instance)
    void spin(Tower &tower);
    void spin_with_select(Tower &tower);

    int stop_spin(bool wait = true);

    [[deprecated]] void spin_join();
    void join_spin();

    void set_spin_cancel_token();

    int start_spin_with_select_realtime(Tower &tower, int abort_on_fault);
    int start_spin_realtime(Tower &tower, int abort_on_fault);
}

#endif
