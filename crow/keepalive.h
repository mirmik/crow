#ifndef CROW_KEEPALIVE_H
#define CROW_KEEPALIVE_H

#include <igris/time/systime.h>
#include <igris/time/timer_manager.h>

namespace crow
{
    extern igris::timer_manager<igris::timer_spec<decltype(igris::millis())>>
        keepalive_timer_manager;
}

#endif