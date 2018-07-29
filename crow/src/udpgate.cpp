#include <g1/gateway.h>
#include <g1/gates/udpgate.h>

g1::udpgate* g1::create_udpgate(uint16_t port, uint8_t id) {
	auto g = new g1::udpgate;
	g->open(port);
	g1::link_gate(g, id);
	return g;
}