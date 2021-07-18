#include <crow/print.h>
#include <crow/tower.h>

#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/string/hexascii_string.h>
#include <igris/util/string.h>

#include <nos/fprint.h>
#include <nos/print.h>

void crow::diagnostic(const char *notation, crow_packet *pack)
{
    bool postfix_points = crow_packet_datasize(pack) > crow::debug_data_size;

    nos::fprint("{}: ("
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
                notation, pack->header.qos, (uint8_t)pack->header.f.ack,
                (uint16_t)pack->header.ackquant, (uint8_t)pack->header.alen,
                (uint16_t)pack->header.flen, (uint8_t)pack->header.f.type,
                pack->header.seqid,
                igris::hexascii_encode(crow_packet_addrptr(pack), crow_packet_addrsize(pack)),
                pack->header.stg, crow_packet_datasize(pack),
                igris::dstring(crow_packet_dataptr(pack),
                               crow_packet_datasize(pack) > crow::debug_data_size
                                   ? crow::debug_data_size
                                   : crow_packet_datasize(pack)));

    if (postfix_points)
        nos::println("...)");
    else
        nos::println(")");
}
