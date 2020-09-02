#include <crow/tower.h>
#include <crow/print.h>
#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/util/string.h>
#include <nos/fprint.h>
#include <nos/print.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
	bool postfix_points = pack->rawdata().size() > crow::debug_data_size;

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
		notation, pack->header.qos, (uint8_t)pack->header.f.ack, (uint16_t)pack->header.ackquant,
		(uint8_t)pack->header.alen, (uint16_t)pack->header.flen,
		(uint8_t)pack->header.f.type,
		pack->header.seqid,
		igris::hexascii_encode(pack->addrptr(), pack->addrsize()),
		pack->header.stg,
		pack->rawdata().size(),
		igris::dstring(pack->rawdata().data(), pack->rawdata().size() > crow::debug_data_size ? crow::debug_data_size : pack->rawdata().size()));

		if (postfix_points) 
			nos::println("...)");
		else
			nos::println(")");
}