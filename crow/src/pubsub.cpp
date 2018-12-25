#include <crow/pubsub.h>
#include <gxx/util/hexer.h>
#include <gxx/print.h>

const uint8_t * brocker_host;
uint8_t brocker_host_len;

/*crow::host brocker_host;
crow::QoS brocker_qos = crow::QoS(0);
uint16_t brocker_ackquant = DEFAULT_ACKQUANT;
*/
void(*crow_pubsub_handler)(crow::packet* pack);

void crow::publish_buffer(const char* theme, const void* data, uint16_t dlen, uint8_t qos, uint16_t acktime)
{
	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = (uint8_t) strlen(theme);
	subps_d.datsz = dlen;

	struct iovec iov[4] =
	{
		{ &subps, sizeof(subps) },
		{ &subps_d, sizeof(subps_d) },
		{ (void*)theme, subps.thmsz },
		{ (void*)data, subps_d.datsz },
	};

	crow::send_v(brocker_host, brocker_host_len, iov, 4, G1_G3TYPE, qos, acktime);
}

void crow::publish(const char* theme, const char* data, uint8_t qos, uint16_t acktime) {
	crow::publish_buffer(theme, data, (uint16_t) strlen(data), qos, acktime);
}

void crow::publish(const char* theme, const std::string& data, uint8_t qos, uint16_t acktime) {
	crow::publish_buffer(theme, data.data(), (uint16_t) data.size(), qos, acktime);
}

void crow::subscribe(const char* theme, uint8_t qos, uint16_t acktime, uint8_t rqos, uint16_t racktime) {
	size_t thmsz = strlen(theme);

	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_control subps_c;

	subps.type = SUBSCRIBE;
	subps.thmsz = (uint8_t) thmsz;
	subps_c.qos = rqos;
	subps_c.ackquant = racktime;

	struct iovec iov[3] = {
		{ &subps, sizeof(subps) },
		{ &subps_c, sizeof(subps_c) },
		{ (void*)theme, thmsz },
	};

	crow::send_v(brocker_host, brocker_host_len, iov, 3, G1_G3TYPE, qos, acktime);
}
/*
void crow::subscribe(const char* theme, crow::QoS qos) {
	crow::subscribe(theme, strlen(theme), qos);
}
*/
void crow::set_publish_host(const uint8_t * hexhost, size_t hsize)
{
	brocker_host = hexhost;
	brocker_host_len = (uint8_t) hsize;
}

std::string crow::envcrowker() 
{
	uint8_t buf[128];
	const char* envcr = getenv("CROWKER");
	auto ss = hexer_s(buf, 128, envcr);
	gxx::print(envcr);
	return std::string((char*)buf, ss);
}

std::string crow::environment_crowker() 
{
	const char* envcr = getenv("CROWKER");
	return std::string(envcr);
}

void crow::set_crowker(const std::string& crowker)  
{
	uint8_t buf[128];
	auto ss = hexer_s(buf, 128, crowker.c_str());
	brocker_host = (uint8_t*) malloc(ss);
	memcpy((void*)brocker_host, (void*)buf, ss);
	brocker_host_len = ss;
}



/*
void crow::set_publish_qos(crow::QoS qos) {
	brocker_qos = qos;
}

gxx::buffer crow::pubsub_message_datasect(crow::packet* pack) {
	auto shps = crow::get_subheader_pubsub(pack);
	auto shps_d = crow::get_subheader_pubsub_data(pack);
	return gxx::buffer(pack->dataptr() + sizeof(subheader_pubsub) + sizeof(subheader_pubsub_data) + shps->thmsz, shps_d->datsz);
}*/