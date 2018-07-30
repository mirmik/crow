#include <crow/pubsub.h>

void crow::incoming_pubsub_packet(crow::packet* pack) {
	gxx::println("crow::incoming_pubsub_packet");
}