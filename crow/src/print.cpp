#include <crow/tower.h>
#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/util/string.h>
#include <nos/fprint.h>
#include <nos/print.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
	bool postfix_points = pack->rawdata().size() > 16;

	nos::fprint(
		"{}: ("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"flen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"dlen:{}, "
		"data:{}",
		notation, pack->header.qos, (uint8_t)pack->header.f.ack,
		(uint8_t)pack->header.alen, (uint16_t)pack->header.flen,
		(uint8_t)pack->header.f.type,
		igris::hexascii_encode(pack->addrptr(), pack->addrsize()),
		pack->header.stg,
		pack->rawdata().size(),
		igris::dstring(pack->rawdata().data(), pack->rawdata().size() > 16 ? 16 : pack->rawdata().size()));

		if (postfix_points) 
			nos::println("...)");
		else
			nos::println(")");
}