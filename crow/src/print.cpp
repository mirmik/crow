#include <crow/tower.h>
#include <igris/util/dstring.h>
#include <igris/util/hexascii.h>
#include <igris/util/string.h>
#include <nos/fprint.h>
#include <nos/print.h>

/*void crow_print_to(igris::io::ostream& out, crow::packet* pack) {
	igris::fprint_to(out, "("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}, "
		"released:{}"
		")",
		pack->header.qos,
		(uint8_t)pack->header.f.ack,
		pack->header.alen,
		(uint8_t)pack->header.f.type,
		igris::hexascii_encode((const uint8_t*)pack->addrptr(),
pack->header.alen), pack->header.stg, igris::dstring(pack->data()), pack->flags
	);
}

void crow_print(crow::packet* pack) {
	crow_print_to(*igris::standart_output, pack);
}

void crow_println(crow::packet* pack) {
	crow_print_to(*igris::standart_output, pack);
	igris::print_to(*igris::standart_output, "\n");
}*/

void crow::diagnostic(const char *notation, crow::packet *pack)
{
	nos::fprintln(
		"{}: ("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"flen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}"
		")",
		notation, pack->header.qos, (uint8_t)pack->header.f.ack,
		(uint8_t)pack->header.alen, (uint16_t)pack->header.flen,
		(uint8_t)pack->header.f.type,
		igris::hexascii_encode(pack->addrptr(), pack->addrsize()),
		pack->header.stg,
		igris::dstring(pack->rawdata().data(), pack->rawdata().size()));
}