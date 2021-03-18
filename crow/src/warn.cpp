#include <crow/warn.h>
#include <igris/dprint.h>

void crow::warn(igris::buffer msg)
{
    dpr("warn: ");
    debug_write(msg.data(), msg.size());
    dln();
}