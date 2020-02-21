#include <crow/tower.h>
#include <crow/print.h>
#include <igris/dprint.h>

void crow::diagnostic(const char *notation, crow::packet *pack)
{
	//bool postfix_points = pack->rawdata().size() > 16;

	/*nos::fprint(
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
			nos::println(")");*/

	dpr(notation);
	dpr(": (");
	dpr("ptr:");dprptr(pack);
	dpr("qos:");dpr(pack->header.qos);
	dpr(",ack:");dpr((uint8_t)pack->header.f.ack);
	dpr(",atim:");dpr((uint16_t)pack->header.ackquant);
	dpr(",alen:");dpr((uint8_t)pack->header.alen);
	dpr(",flen:");dpr((uint8_t)pack->header.flen);
	dpr(",type:");dpr((uint8_t)pack->header.f.type);
	dpr(",addr:");debug_writehex(pack->addrptr(), pack->addrsize());
	dpr(",stg:");dpr(pack->header.stg);
	dpr(",rescount:");dpr((uint8_t)pack->_ackcount);
	dpr(",data:");dpr(pack->rawdata().slice(0,20));
	dprln(")");
}