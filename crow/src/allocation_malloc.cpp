/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

int crow::allocated_count = 0;

void crow::deallocate_packet(crow::packet *pack) 
{ 
	if (pack)
		allocated_count--;
	free(pack);
}

crow::packet *crow::allocate_packet(size_t adlen)
{
	allocated_count++;
	return (crow::packet *)malloc(adlen + sizeof(crow::packet));
}
