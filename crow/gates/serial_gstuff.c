#include <crow/gates/serial_gstuff.h>

struct crow_gw* crow_create_serial_gstuff(const char* path, uint32_t baudrate, uint8_t id) 
{
	struct crow_serial_gstuff* g = (struct crow_serial_gstuff*) 
		malloc(sizeof(struct crow_serial_gstuff));
	
	//crow_serialgate_open(g, port); // TODO: should return NULL on error
	crow_link_gate(&g->gw, id);
	
	return &g->gw;
}

void crow_serialgate_send(struct crow_gw* g, struct crowket* pack) {
	/*std::string str;
	gxx::io::std_string_writer strm(str);
	gxx::gstuff::sender sender(strm);

	sender.start_message();
	sender.write((char*)&pack->header, pack->header.flen);
	sender.end_message();

	mtx.lock();
	ser->write((uint8_t*)str.data(), str.size());
	mtx.unlock();

	crow::return_to_tower(pack, crow::status::Sended);*/
}

void crow_serial_gstuff_send(struct crow_gw* gw, struct crowket* pack) {
	
}

void crow_serial_gstuff_nblock_onestep(struct crow_gw* gw) {

}

const struct crow_gw_operations crow_serial_gstuff_ops = {
	.send = crow_serial_gstuff_send,
	.nblock_onestep = crow_serial_gstuff_nblock_onestep
};
