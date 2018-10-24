#include <crow/tower.h>
#include <gxx/print.h>
#include <gxx/print/stdprint.h>
#include <gxx/util/hexascii.h>
#include <gxx/util/string.h>

void crow_print_to(gxx::io::ostream& out, crowket_t* pack) {
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
		gxx::hexascii_encode((const uint8_t*)crowket_addrptr(pack), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(crowket_dataptr(pack), crowket_datasize(pack)), 
		pack->flags
	);
}

void crow_print(crowket_t* pack) {
	crow_print_to(*gxx::standart_output, pack);
}

void crow_println(crowket_t* pack) {
	crow_print_to(*gxx::standart_output, pack);
	gxx::print_to(*gxx::standart_output, "\n");
}

void crow_diagnostic(const char* notation, crowket_t* pack) 
{
	gxx::fprintln("{}: ("
		"qos:{}, "
		"ack:{}, "
		"alen:{}, "
		"type:{}, "
		"addr:{}, "
		"stg:{}, "
		"data:{}, "
		")", 
		notation,
		pack->header.qos,
		(uint8_t)pack->header.f.ack, 
		pack->header.alen, 
		(uint8_t)pack->header.f.type, 
		gxx::hexascii_encode((const uint8_t*)crowket_addrptr(pack), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(crowket_dataptr(pack), crowket_datasize(pack))
	);
}