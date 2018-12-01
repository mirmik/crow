#include <crow/tower.h>
#include <gxx/print.h>
#include <gxx/print/stdprint.h>
#include <gxx/util/hexascii.h>
#include <gxx/util/string.h>

/*void crow_print_to(gxx::io::ostream& out, crow::packet* pack) {
	gxx::fprint_to(out, "("
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
		gxx::hexascii_encode((const uint8_t*)pack->addrptr(), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(pack->data()), 
		pack->flags
	);
}

void crow_print(crow::packet* pack) {
	crow_print_to(*gxx::standart_output, pack);
}

void crow_println(crow::packet* pack) {
	crow_print_to(*gxx::standart_output, pack);
	gxx::print_to(*gxx::standart_output, "\n");
}*/

void crow::diagnostic(const char* notation, crow::packet* pack) 
{
	gxx::fprintln("{}: ("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"flen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}"
		")", 
		notation,
		pack->header.qos,
		(uint8_t)pack->header.f.ack, 
		(uint8_t)pack->header.alen, 
		(uint16_t)pack->header.flen, 
		(uint8_t)pack->header.f.type, 
		gxx::hexascii_encode(pack->addr()), 
		pack->header.stg, 
		gxx::dstring(pack->rawdata())
	);
}