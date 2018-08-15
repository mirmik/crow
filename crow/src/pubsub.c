#include <crow/pubsub.h>

/*crow::host brocker_host;
crow::QoS brocker_qos = crow::QoS(0);
uint16_t brocker_ackquant = DEFAULT_ACKQUANT;
*/
void(*crow_pubsub_handler)(crow_packet_t* pack);
/*
void crow::publish(const void* theme, size_t thmsz, const void* data, size_t datsz) {
	crow::subheader_pubsub subps;
	crow::subheader_pubsub_data subps_d;

	subps.type = crow::frame_type::PUBLISH;
	subps.thmsz = thmsz;
	subps_d.datsz = datsz;

	gxx::iovec iov[4] = {
		{ &subps, sizeof(subps) },
		{ &subps_d, sizeof(subps_d) },
		{ theme, thmsz },
		{ data, datsz },
	};

	crow::send(brocker_host.data, brocker_host.size, iov, 4, G1_G3TYPE, brocker_qos, DEFAULT_ACKQUANT);
}

void crow::publish(const char* theme, const char* data) {
	crow::publish(theme, strlen(theme), data, strlen(data));
}

void crow::subscribe(const void* theme, size_t thmsz, crow::QoS qos) {
	crow::subheader_pubsub subps;
	crow::subheader_pubsub_control subps_c;

	subps.type = crow::frame_type::SUBSCRIBE;
	subps.thmsz = thmsz;
	subps_c.qos = qos;
	subps_c.ackquant = brocker_ackquant;

	gxx::iovec iov[4] = {
		{ &subps, sizeof(subps) },
		{ &subps_c, sizeof(subps_c) },
		{ theme, thmsz },
	};

	crow::send(brocker_host.data, brocker_host.size, iov, 3, G1_G3TYPE, crow::QoS(2));	
}

void crow::subscribe(const char* theme, crow::QoS qos) {
	crow::subscribe(theme, strlen(theme), qos);
}


void crow::set_publish_host(const crow::host& host) { 
	brocker_host = host; 
}

void crow::set_publish_qos(crow::QoS qos) { 
	brocker_qos = qos; 
}

gxx::buffer crow::pubsub_message_datasect(crow::packet* pack) {
	auto shps = crow::get_subheader_pubsub(pack);
	auto shps_d = crow::get_subheader_pubsub_data(pack);
	return gxx::buffer(pack->dataptr() + sizeof(subheader_pubsub) + sizeof(subheader_pubsub_data) + shps->thmsz, shps_d->datsz);	
}*/