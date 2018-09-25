#include <crow/tower.h>
#include <crow/gates/udpgate.h>
#include <gxx/util/hexer.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

uint8_t addr[128];
int addrsize;

uint8_t qos = 0;
bool raw;
bool packmon;
bool sniffer;
bool echo;
bool dump;

//gxx::log::colored_stdout_target console_target;

void incoming_handler(crow_packet_t* pack) {
	dpr("incoming: \n"); 
	if (echo) {
		crow_send(crow_packet_addrptr(pack), pack->header.alen, crow_packet_dataptr(pack), crow_packet_datasize(pack), 0, 0, 300);
	}
	
	if (packmon) {	
		crow_print(pack); 
		//gxx::println();
	} else {
		//gxx::println(gxx::dstring(crow_packet_dataptr(pack), crow_packet_datasize(pack)));
	}
	
	crow_release(pack);
}

void traveling_handler(crow_packet_t* pack) {
	printf("travel\n");
}

void transit_handler(crow_packet_t* pack) {
	if (sniffer) {
		dpr("transit: ");
		crow_print(pack); 
	}
}

void* console_listener(void* arg) {
	(void) arg;

  	char* input;
  	int len;

	rl_catch_signals = 0;
  	while(1) {	
 	   input = readline("");
    	
    	if (!input)
    	  break;
		add_history(input);
	
		len = strlen(input);
		input[len] = '\n';
		input[len + 1] = '\0';
		crow_send(addr, addrsize, input, len+1, 0, qos, 200);
	}

	exit(0);
}

uint16_t udpport = 0;
//std::string serial_port;
//int serialfd;

//void signal_handler(int sig) {
//	exit(sig);
//}

int main(int argc, char* argv[]) {
	pthread_t console_thread;

	//signal(SIGINT, signal_handler);

	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'},
		{"qos", required_argument, NULL, 'q'},
		{"serial", required_argument, NULL, 'S'},
		{"sniffer", no_argument, NULL, 's'},
		{"pack", no_argument, NULL, 'v'},
		{"echo", no_argument, NULL, 'e'},
		{"dump", no_argument, NULL, 'd'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "uvSsedq", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'q': qos = atoi(optarg); break;
			case 'u': udpport = atoi(optarg); break;
			//case 'S': serial_port = optarg; break;
			case 's': sniffer = true; break;
			case 'v': packmon = true; break;
			case 'e': echo = true; break;
			case 'd': dump = true; break;
			case 0: break;
		}
	}

	if (crow_create_udpgate(udpport, G1_UDPGATE) == NULL) {
		perror("udpgate open");
		exit(-1);
	}
		
	crow_user_incoming_handler = incoming_handler;
	crow_traveling_handler = traveling_handler;
	crow_transit_handler = transit_handler;
	
	/*if (!serial_port.empty()) {
		if (crow_create_serialgate(serial_port.c_str(), 115200, 42) == NULL) {
			perror("serialgate open");
			exit(-1);
		}				
		//auto ser = new serial::Serial(serial_port, 115200);
		//auto* serial = new gxx::io::file(ser->fd());
		//auto* serialgate = new crow::serial_gstuff_gate(ser);
		//crow_link_gate(serialgate, 42);
	}*/

	if (optind < argc) {
		addrsize = hexer(addr, 128, argv[optind], strlen(argv[optind]));
		if (addrsize < 0) {
			dprln("Wrong address format");
			exit(-1);
		}
	} else {
		addrsize = 0;
	}	
		
	if (pthread_create(&console_thread, NULL, console_listener, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	crow_spin();
}

