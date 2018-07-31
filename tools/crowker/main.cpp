#include <crow/tower.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

int udpport = -1;
bool sniffer_option = false;

void sniffer_travel_handler(crow::packet* pack) {
	gxx::print("travel: "); 
	crow::println(pack);
}

int main(int argc, char* argv[]) {
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

