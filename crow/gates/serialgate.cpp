#include <crow/gates/serial_gstuff.h>
#include <gxx/serial/serial.h>


crow_gw_t* crow_create_serialgate(const char* path, uint32_t baudrate, uint8_t id) {
	crow_serialgate_t* g = (crow_serialgate*) malloc(sizeof(crow_serialgate));
	//crow_serialgate_open(g, port); // TODO: should return NULL on error
	crow_link_gate(&g->gw, id);
	return &g->gw;
}