#include <crow/tower.h>
#include <crow/gates/udpgate.h>
#include <crow/gates/serial_gstuff.h>
#include <gxx/util/hexer.h>
#include <gxx/io/file.h>
#include <gxx/serial/serial.h>

#include <gxx/print/stdprint.h>

#include <thread>

#include <iostream>
#include <getopt.h>

uint8_t addr[128];
int addrsize;

uint8_t qos = 0;
uint8_t type = 0;

bool raw;
bool packmon;
bool sniffer;
bool echo;
bool dump;
bool api;

bool pulse = false;
std::string pulse_data;

//gxx::log::colored_stdout_target console_target;

void incoming_handler(crow::packet* pack) {
	if (api && pack->header.type == 15 && pack->datasect() == "exit") {
		gxx::println("ctrans finished by remote host");
		exit(0);
	} 

	if (echo) {
		crow::send(pack->addrptr(), pack->header.alen, pack->dataptr(), pack->datasize(), pack->header.type, pack->header.qos, pack->header.ackquant);
	}
	gxx::print("incoming: "); 
	
	if (packmon) {	
		if (dump) crow::print_dump(pack);
		else crow::print(pack); 
		gxx::println();
	} else {
		gxx::println(gxx::dstring(pack->dataptr(), pack->datasize()));
	}
	
	crow::release(pack);
}

void traveling_handler(crow::packet* pack) {}

void transit_handler(crow::packet* pack) {
	if (sniffer) {
		gxx::print("transit: ");
		if (dump) crow::print_dump(pack);
		else crow::print(pack); 
		gxx::println();
	}
}

void console_listener() {
	std::string in;
	while(1) {
		std::getline(std::cin, in);
		in += '\n';
		crow::send(addr, addrsize, in.data(), in.size(), type, (crow::QoS) qos, 200);
	}
}

void pulse_thread() {
	auto pack = crow::no_release_send(addr, addrsize, pulse_data.data(), pulse_data.size(), type, (crow::QoS) qos, 200);
	while (pack -> released_by_tower != true);
	crow::release(pack);
	exit(0);
}

int udpport = -1;
std::string serial_port;
int serialfd;

int main(int argc, char* argv[]) {
	const struct option long_options[] = {
		{"qos", required_argument, NULL, 'q'},
		{"type", required_argument, NULL, 't'},
		{"udp", required_argument, NULL, 'u'},
		{"serial", required_argument, NULL, 'S'},
		{"sniffer", no_argument, NULL, 's'},
		{"pack", no_argument, NULL, 'v'},
		{"echo", no_argument, NULL, 'e'},
		{"dump", no_argument, NULL, 'd'},
		{"api", no_argument, NULL, 'a'},
		{"pulse", required_argument, NULL, 'p'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "uvSsedap", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'q': qos = atoi(optarg); break;
			case 't': type = atoi(optarg); break;
			case 'u': udpport = atoi(optarg); break;
			case 'S': serial_port = optarg; break;
			case 's': sniffer = true; break;
			case 'v': packmon = true; break;
			case 'e': echo = true; break;
			case 'd': dump = true; break;
			case 'a': api = true; break;
			case 'p': pulse_data = optarg; pulse = true; break;
			case 0: break;
		}
	}

	crow::udpgate udpgate;
	if (udpport != -1) { 
		int ret = udpgate.open(udpport);
		if (ret < 0) {
			perror("udpgate open");
			exit(-1);
		}
	}
	else udpgate.open();

	crow::user_incoming_handler = incoming_handler;
	crow::traveling_handler = traveling_handler;
	crow::transit_handler = transit_handler;
	crow::link_gate(&udpgate, G1_UDPGATE);
	
	if (!serial_port.empty()) {
		auto ser = new serial::Serial(serial_port, 115200);
		//auto* serial = new gxx::io::file(ser->fd());
		auto* serialgate = new crow::serial_gstuff_gate(ser);
		crow::link_gate(serialgate, 42);
	}

	if (optind < argc) {
		addrsize = hexer(addr, 128, argv[optind], strlen(argv[optind]));
		if (addrsize < 0) {
			gxx::println("Wrong address format");
			exit(-1);
		}	
		
		if (!pulse) {
			auto thr = new std::thread(console_listener);
			thr->detach();
		}
		else {
			auto thr = new std::thread(pulse_thread);
			thr->detach();
		}
	}

	crow::spin();
}

