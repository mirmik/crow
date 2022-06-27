#include <crow/keepalive.h>
#include <igris/time/systime.h>
#include <igris/time/timer_manager.h>

igris::timer_manager<igris::timer_spec<decltype(
    igris::millis())>> crow::keepalive_timer_manager(&igris::millis);