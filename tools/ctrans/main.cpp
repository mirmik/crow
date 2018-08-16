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

bool raw;
bool packmon;
bool sniffer;
bool echo;
bool dump;

//gxx::log::colored_stdout_target console_target;

void incoming_handler(crow_packet_t* pack) {
	if (echo) {
		crow_send(crow_packet_addrptr(pack), pack->header.alen, crow_packet_dataptr(pack), crow_packet_datasize(pack), 0, 0, 300);
	}
	//gxx::print("incoming: "); 
	
	/*if (packmon) {	
		if (dump) crow::print_dump(pack);
		else crow::print(pack); 
		gxx::println();
	} else {
		gxx::println(gxx::dstring(pack->dataptr(), pack->datasize()));
	}*/
	
	crow_release(pack);
}

void traveling_handler(crow_packet_t* pack) {}

void transit_handler(crow_packet_t* pack) {
	if (sniffer) {
		//gxx::print("transit: ");
		//if (dump) crow::print_dump(pack);
		//else crow::print(pack); 
		//gxx::println();
	}
}

void console_listener() {
	std::string in;
	while(1) {
		std::getline(std::cin, in);
		in += '\n';
		crow_send(addr, addrsize, in.data(), in.size(), 0, 0, 200);
	}
}

uint16_t udpport = 0;
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

	crow_gw_t* udpgate = crow_create_udpgate(udpport, G1_UDPGATE);
	if (udpgate == NULL) {
		perror("udpgate open");
		exit(-1);
	}
		
	crow_user_incoming_handler = incoming_handler;
	crow_traveling_handler = traveling_handler;
	crow_transit_handler = transit_handler;
	crow_link_gate(udpgate, G1_UDPGATE);
	
	if (!serial_port.empty()) {
		auto ser = new serial::Serial(serial_port, 115200);
		//auto* serial = new gxx::io::file(ser->fd());
		auto* serialgate = new crow::serial_gstuff_gate(ser);
		crow_link_gate(serialgate, 42);
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

	crow_spin();
}

