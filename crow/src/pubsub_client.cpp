#include <crow/pubsub.h>

crow::host brocker_host;
crow::QoS brocker_qos = crow::QoS(0);
uint16_t brocker_ackquant = DEFAULT_ACKQUANT;

void(*crow::subscribe_handler)(crow::packet* pack);

void crow::incoming_pubsub_packet(crow::packet* pack) {
	//crow::println(pack);
	if (subscribe_handler) subscribe_handler(pack);
	else crow::release(pack);
}

void crow::publish(const char* theme, size_t thmsz, const char* data, size_t datsz) {
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

void crow::subscribe(const char* theme, size_t thmsz) {
	crow::subheader_pubsub subps;
	crow::subheader_pubsub_control subps_c;

	subps.type = crow::frame_type::SUBSCRIBE;
	subps.thmsz = thmsz;
	subps_c.qos = brocker_qos;
	subps_c.ackquant = brocker_ackquant;

	gxx::iovec iov[4] = {
		{ &subps, sizeof(subps) },
		{ &subps_c, sizeof(subps_c) },
		{ theme, thmsz },
	};

	crow::send(brocker_host.data, brocker_host.size, iov, 3, G1_G3TYPE, brocker_qos);	
}


void crow::set_publish_host(const crow::host& host) { brocker_host = host; }
void crow::set_publish_qos(crow::QoS qos) { brocker_qos = qos; }