#include <crow/gates/serial_gstuff.h>

void callback_handler(char * buf, int len);

struct crow_gw* crow_create_serial_gstuff(const char* path, uint32_t baudrate, uint8_t id) 
{
	struct crow_serial_gstuff* g = (struct crow_serial_gstuff*) 
		malloc(sizeof(struct crow_serial_gstuff));
	
	//crow_serialgate_open(g, port); // TODO: should return NULL on error
	g->rpack = NULL;
	gstuff_automate_init(buf);

	crow_link_gate(&g->gw, id);
	
	return &g->gw;
}

void callback_handler(char * buf, int len) {
	struct crowket * block = rpack;
	init_recv();

	block->revert_stage(id);

	crowket_initialization(block, this);
	crow_travel(block);
}

void crow_serial_gstuff_send(struct crow_gw* gw, struct crowket* pack) 
{
	struct crow_serial_gstuff* g = mcast_out(gw, struct crow_serial_gstuff, gw);
	/*std::string str;
	gxx::io::std_string_writer strm(str);
	gxx::gstuff::sender sender(strm);

	sender.start_message();
	sender.write((char*)&pack->header, pack->header.flen);
	sender.end_message();

	mtx.lock();
	ser->write((uint8_t*)str.data(), str.size());
	mtx.unlock();
*/
	crow_return_to_tower(pack);
}

/*void crow_serial_gstuff_send(struct crow_gw* gw, struct crowket* pack) 
{

}*/

static inline void 
init_recv(struct crow_serial_gstuff* g) {
	g->rpack = (struct crowket*) malloc(128 + sizeof(struct crowket) - sizeof(struct crowket_header));
	gstuff_automat_init(&g->recver, (char*)&rpack->header, 128);
	//recver.init(gxx::buffer((char*)&rpack->header, 128));
}

void crow_serial_gstuff_nblock_onestep(struct crow_gw* gw) 
{
	if (rpack == nullptr) {
		init_recv();
	}

	char c;
	//int len = read(ser->fd(), (uint8_t*)&c, 1);
	int len = ser->read((uint8_t*)&c, 1);
	if (len == 1) {
		//dprhex(c); dpr("\t"); gxx::println(gxx::dstring(&c, 1));
		recver.newchar(c);
	}
}

const struct crow_gw_operations crow_serial_gstuff_ops = {
	.send = crow_serial_gstuff_send,
	.nblock_onestep = crow_serial_gstuff_nblock_onestep
};
