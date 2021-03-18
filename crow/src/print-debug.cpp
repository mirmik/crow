#include <crow/print.h>
#include <crow/tower.h>
#include <igris/dprint.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
    dpr(notation);
    dpr(": (");
    dpr("ptr:");
    dprptr(pack);
    dpr("qos:");
    dpr(pack->header.qos);
    dpr(",ack:");
    dpr((uint8_t)pack->header.f.ack);
    dpr(",atim:");
    dpr((uint16_t)pack->header.ackquant);
    dpr(",alen:");
    dpr((uint8_t)pack->header.alen);
    dpr(",flen:");
    dpr((uint8_t)pack->header.flen);
    dpr(",type:");
    dpr((uint8_t)pack->header.f.type);
    dpr(",addr:");
    debug_writehex(pack->addrptr(), pack->addrsize());
    dpr(",stg:");
    dpr(pack->header.stg);
    dpr(",rescount:");
    dpr((uint8_t)pack->_ackcount);
    dpr(",data:");
    dpr(pack->rawdata().slice(0, 20));
    dprln(")");
}
