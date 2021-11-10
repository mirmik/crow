#include <crow/print.h>
#include <crow/tower.h>

#include <igris/string/hexascii_string.h>
#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/util/string.h>

#include <nos/fprint.h>
#include <nos/print.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
    bool postfix_points = pack->datasize() > crow::debug_data_size;

    nos::fprint(
        "{}: ("
        "qos:{}, "
        "ack:{}, "
        "atim:{}, "
        "alen:{}, "
        "flen:{}, "
        "type:{}, "
        "seqid:{}, "
        "addr:{}, "
        "stg:{}, "
        "dlen:{}, "
        "data:{}",
        notation, pack->header().qos, (uint8_t)pack->header().u.f.ack,
        (uint16_t)pack->header().ackquant, (uint8_t)pack->header().alen,
        (uint16_t)pack->header().flen, (uint8_t)pack->header().u.f.type,
        pack->header().seqid,
        igris::hexascii_encode(pack->addrptr(),
                               pack->addrsize()),
        pack->header().stg, pack->datasize(),
        igris::dstring(pack->dataptr(),
                       pack->datasize() > crow::debug_data_size
                           ? crow::debug_data_size
                           : pack->datasize()));

    if (postfix_points)
        nos::println("...)");
    else
        nos::println(")");
}
