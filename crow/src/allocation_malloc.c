/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

void crow_deallocate_packet(crowket_t* pack) {  
	free(pack); 
}

crowket_t* crow_allocate_packet(size_t adlen) {
	return (crowket_t*) malloc(adlen + sizeof(crowket_t));
}

