#include <crow/tower.h>

/*void crow::utilize_block(crow::packet_header* block) { 
	crow::logger.debug("utilize_block (ptr:{})", block);
	free(block); 
}
*/
void crow::utilize_packet(crow::packet* pack) {  
	//crow::logger.debug("utilize_packet (ptr:{})", pack);
	free(pack); 
}
/*
crow::packet_header* crow::allocate_block(uint8_t alen, uint16_t dlen) {
	auto size =  sizeof(crow::packet_header) + alen + dlen;
	auto pack = (packet_header*)malloc(size);
	crow::logger.debug("allocate_block (ptr:{}, size:{})", pack, size);
	return pack;
}

crow::packet* crow::allocate_packet() {  
	auto pack = (packet*)malloc(sizeof(crow::packet));
	crow::logger.debug("allocate_packet (ptr:{}, size:{})", pack, sizeof(crow::packet));
	return pack;
}*/

crow::packet* crow::allocate_packet(size_t adlen) {
	auto size = adlen + sizeof(crow::packet);
	auto pack = (packet*) malloc(size);
	//crow::logger.debug("allocate_packet (ptr:{}, size:{})", pack, size);
	return pack;
}

