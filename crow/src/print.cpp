#include <crow/tower.h>
#include <gxx/print.h>
#include <gxx/print/stdprint.h>
#include <gxx/util/hexascii.h>
#include <gxx/util/string.h>

void crow_print_to(gxx::io::ostream& out, crow_packet_t* pack) {
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
		(uint8_t)pack->header.ack, 
		pack->header.alen, 
		(uint8_t)pack->header.type, 
		gxx::hexascii_encode((const uint8_t*)crow_packet_addrptr(pack), pack->header.alen), 
		pack->header.stg, 
		gxx::dstring(crow_packet_dataptr(pack), crow_packet_datasize(pack)), 
		pack->flags
	);
}

void crow_print(crow_packet_t* pack) {
	crow_print_to(*gxx::standart_output, pack);
}

void crow_println(crow_packet_t* pack) {
	crow_print_to(*gxx::standart_output, pack);
	gxx::print_to(*gxx::standart_output, "\n");
}
