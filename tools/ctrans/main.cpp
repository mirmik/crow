#include <crow/tower.h>
#include <crow/indexes.h>
#include <crow/gates/udpgate.h>
#include <crow/gates/serial_gstuff.h>
#include <gxx/log/target2.h>
#include <gxx/util/hexer.h>
#include <gxx/io/file.h>
#include <gxx/serial/serial.h>

#include <gxx/print/stdprint.h>

#include <thread>

#include <iostream>
#include <getopt.h>

uint8_t addr[128];
int addrsize;

bool raw;
bool packmon;
bool sniffer;
bool echo;
bool dump;

gxx::log::colored_stdout_target console_target;

void incoming_handler(crow::packet* pack) {
	if (echo) {
		crow::send(pack->addrptr(), pack->header.alen, pack->dataptr(), pack->datasize());
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
		crow::send(addr, addrsize, in.data(), in.size(), 0, (crow::QoS) 0);
	}
}

int udpport = -1;
std::string serial_port;
int serialfd;

int main(int argc, char* argv[]) {
	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{"serial", required_argument, NULL, 'S'},
		{"sniffer", no_argument, NULL, 's'},
		{"pack", no_argument, NULL, 'v'},
		{"echo", no_argument, NULL, 'e'},
		{"dump", no_argument, NULL, 'd'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "uvSsed", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'u': udpport = atoi(optarg); break;
			case 'S': serial_port = optarg; break;
			case 's': sniffer = true; break;
			case 'v': packmon = true; break;
			case 'e': echo = true; break;
			case 'd': dump = true; break;
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
		auto thr = new std::thread(console_listener);
		thr->detach();
	}

	crow::spin();
}

