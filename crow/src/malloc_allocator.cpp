/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>

void crow::utilize_packet(crow::packet* pack) {  
	free(pack); 
}

crow::packet* crow::allocate_packet(size_t adlen) {
	auto size = adlen + sizeof(crow::packet);
	auto pack = (packet*) malloc(size);
	return pack;
}

