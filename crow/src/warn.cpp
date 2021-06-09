#include <crow/warn.h>
#include <igris/dprint.h>

void crow::warn(std::string_view msg)
{
    dpr("warn: ");
    debug_write(msg.data(), msg.size());
    dln();
}