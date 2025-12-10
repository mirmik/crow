#include <crow/print.h>
#include <crow/tower.h>
#include <crow/proto/node.h>

#include <igris/string/hexascii_string.h>
#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/util/string.h>

#include <nos/fprint.h>
#include <nos/print.h>
#include <nos/io/stdfile.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
    bool postfix_points = pack->datasize() > crow::debug_data_size;

    nos::fprint_to(nos::cerr,
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
        notation, pack->quality(), pack->ack(),
        pack->ackquant(), pack->addrsize(),
        pack->full_length(), pack->type(),
        pack->seqid(),
        igris::hexascii_encode(pack->addrptr(),
                               pack->addrsize()),
        pack->stage(), pack->datasize(),
        igris::dstring(pack->dataptr(),
                       pack->datasize() > crow::debug_data_size
                           ? crow::debug_data_size
                           : pack->datasize()));

    if (pack->type() == CROW_NODE_PROTOCOL)
    {
        nos::fprint_to(nos::cerr,
        ", sid:{}, "
        "rid:{}",
        crow::node_protocol_cls::sid(pack), crow::node_protocol_cls::rid(pack)
        );
    }

    if (postfix_points)
        nos::println_to(nos::cerr, "...)");
    else
        nos::println_to(nos::cerr, ")");
}
