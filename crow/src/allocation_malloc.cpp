/**
 * @file
 * @author mirmik
 * @brief Управление памятью пакетов через libc malloc
 */

#include <crow/packet.h>
#include <stdlib.h>

#include <igris/sync/syslock.h>
#include <igris/dtrace.h>

int crow::allocated_count = 0;

void crow::deallocate_packet(crow::packet *pack) 
{ 
//	system_lock();
	if (pack)
		allocated_count--;
	free(pack);
//	system_unlock();
}

crow::packet *crow::allocate_packet(size_t adlen)
{
//	system_lock();
	allocated_count++;
	crow::packet * ret = (crow::packet *)malloc(adlen + sizeof(crow::packet));
//	system_unlock();
	return ret;
}
