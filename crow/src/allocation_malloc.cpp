/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

void crow::deallocate_packet(crow::packet* pack) {  
	free(pack); 
}

crow::packet* crow::allocate_packet(size_t adlen) {
	return (crow::packet*) malloc(adlen + sizeof(crow::packet));
}

