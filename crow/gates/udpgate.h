/**
	@file
*/

#ifndef G1_GATES_UDPGATE_H
#define G1_GATES_UDPGATE_H

#include <crow/gateway.h>

__BEGIN_DECLS

//void crow_udpgate_send(crow_gw_t* gw, crowket_t* pack);
//void crow_udpgate_nblock_onestep(crow_gw_t* gw);

//int crow_udpgate_open(crow_udpgate_t* gw, uint16_t port);
crow_gw_t* crow_create_udpgate(uint16_t port, uint8_t id);

__END_DECLS

#endif