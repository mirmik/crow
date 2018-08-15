/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

void crow_deallocate_packet(crow_packet_t* pack) {  
	free(pack); 
}

crow_packet_t* crow_allocate_packet(size_t adlen) {
	return (crow_packet_t*) malloc(adlen + sizeof(crow_packet_t));
}

