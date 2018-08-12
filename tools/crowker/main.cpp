#include <crow/tower.h>
#include <crow/brocker.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

#include <unordered_map>
#include <gxx/print/stdprint.h>

int udpport = -1;
bool sniffer_option = false;
std::unordered_map<std::string, crow::theme> themes;

void sniffer_travel_handler(crow::packet* pack) {
	gxx::print("travel: "); 
	crow::println(pack);
}

void brocker_publish(const std::string& theme, const std::string& data) {
	int subs = 0;

	try {
		auto& thm = themes.at(theme);
		thm.publish(data);
		subs = thm.subs.size();
	} catch (std::exception ex) {
		subs = 0;
	}

	gxx::fprintln("PUBLISH: (theme: {}, data: {}, subs: {})", theme, data, subs);
}

void g3_brocker_subscribe(uint8_t* raddr, size_t rlen, const std::string& theme, crow::QoS qos, uint16_t ackquant) {
	crow::host host(raddr, rlen);

	if (themes.count(theme) == 0) {
		themes[theme] = crow::theme(theme);
	}

	auto& thm = themes[theme];
	thm.subs.emplace(host, true, qos, ackquant);

	gxx::fprintln("G3_SUBSCRIBE: (theme: {}, raddr: {})", theme, gxx::hexascii_encode(raddr, rlen));
}


void incoming_pubsub_packet(crow::packet* pack) {
	auto shps = crow::get_subheader_pubsub(pack);

	switch(shps->type) {
		case crow::frame_type::PUBLISH: {
			//gxx::println("crow::frame_type::PUBLISH");
			auto shps_d = crow::get_subheader_pubsub_data(pack);
			std::string theme(pack->dataptr() + sizeof(crow::subheader_pubsub) + sizeof(crow::subheader_pubsub_data), shps->thmsz);
			std::string data(pack->dataptr() + sizeof(crow::subheader_pubsub) + sizeof(crow::subheader_pubsub_data) + shps->thmsz, shps_d->datsz);
			brocker_publish(theme, data);			
		} break;
		case crow::frame_type::SUBSCRIBE: {
			//gxx::println("crow::frame_type::SUBSCRIBE");
			auto shps_c = crow::get_subheader_pubsub_control(pack);
			std::string theme(pack->dataptr() + sizeof(crow::subheader_pubsub) + sizeof(crow::subheader_pubsub_control), shps->thmsz);
			g3_brocker_subscribe(pack->addrptr(), pack->addrsize(), theme, shps_c->qos, shps_c->ackquant);			
		} break;
		default: {
			gxx::println("unresolved pubsub frame type", (uint8_t)shps->type);
		} break;
	}
	crow::release(pack); 
}

void undelivered_handler(crow::packet* pack) {
	if (pack->header.type == G1_G3TYPE) {
		auto shps = get_subheader_pubsub(pack);

		if (shps->type == crow::frame_type::PUBLISH) {
			std::string theme(pack->dataptr() + sizeof(crow::subheader_pubsub) + sizeof(crow::subheader_pubsub_data), shps->thmsz);
			auto& thm = themes[theme];
			thm.subs.erase(crow::subscriber(crow::host(pack->addrptr(), pack->header.alen), true, pack->header.qos, pack->header.ackquant));
		}
	}
	
	crow::utilize(pack);
}

void crow::theme::publish(const std::string& data) {
	crow::subheader_pubsub subps;
	crow::subheader_pubsub_data subps_d;

	subps.type = crow::frame_type::PUBLISH;
	subps.thmsz = name.size();
	subps_d.datsz = data.size();

	gxx::iovec vec[4] = {
		{ &subps, sizeof(subps) },
		{ &subps_d, sizeof(subps_d) },
		{name.data(), name.size()},
		{data.data(), data.size()}
	};

	for (auto& sub : subs) {
		crow::send(sub.host.data, sub.host.size, vec, 4, G1_G3TYPE, sub.qos, sub.ackquant);
	}
}

int main(int argc, char* argv[]) {
	crow::pubsub_handler = incoming_pubsub_packet;
	crow::undelivered_handler = undelivered_handler;

	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{"debug", no_argument, NULL, 'd'},
		{"sniffer", no_argument, NULL, 's'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "us", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'u': udpport = atoi(optarg); break;
			case 'd': crow::enable_diagnostic(); break;
			case 's': sniffer_option = true; break;
			case 0: break;
		}
	}

	if (sniffer_option) crow::traveling_handler = sniffer_travel_handler;

	crow::udpgate udpgate;
	if (udpport != -1) { 
		int ret = udpgate.open(udpport);
		if (ret < 0) {
			perror("udpgate open");
			exit(-1);
		}
	}
	else {
		gxx::println("you should specify udp port");
		exit(-1);
	}

	crow::link_gate(&udpgate, G1_UDPGATE);

	crow::spin();
}

