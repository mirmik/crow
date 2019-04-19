#include <crow/gates/serial_gstuff.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <crow/hexer.h>
#include <igris/util/dstring.h>

#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <string>

uint8_t addr[128];
int addrsize;

uint8_t qos = 0;
uint8_t type = 0;
uint8_t ackquant = 200;

bool api = false;
bool noconsole = false;
bool noend = false;
bool echo = false;
bool gdebug = false;
bool info = false;

std::string pulse;

void incoming_handler(crow::packet *pack)
{
	// dpr("incoming: ");

	if (echo)
	{
		crow::send(pack->addrptr(), pack->header.alen, pack->dataptr(),
				   pack->datasize(), pack->header.f.type, pack->header.qos,
				   pack->header.ackquant);
	}

	if (api)
	{
		char *dp = pack->dataptr();
		size_t ds = pack->datasize();

		if (strncmp(dp, "exit\n", ds) == 0)
		{
			raise(SIGINT);
		}
	}

	char buf[10000];
	bytes_to_dstring(buf, pack->dataptr(), pack->datasize());
	printf("%s\n", buf);
	fflush(stdout);

	crow::release(pack);
}

/*void traveling_handler(crow::packet* pack) {}

void transit_handler(crow::packet* pack) {
	if (sniffer) {
		dpr("transit: ");
		crow_println(pack);
	}
}*/

void *console_listener(void *arg)
{
	(void)arg;

	char *input;
	size_t len;

	rl_catch_signals = 0;

	while (1)
	{
		input = readline("");

		if (!input)
			break;

		add_history(input);

		len = strlen(input);

		if (!noend)
		{
			input[len] = '\n';
			len++;
		}

		crow::send(addr, (uint8_t)addrsize, input, (uint16_t)len, type, qos,
				   ackquant);
	}

	exit(0);
}

uint16_t udpport = 0;
char *serial_port = NULL;
// std::string serial_port;
// int serialfd;

// void signal_handler(int sig) {
//	printf("dadas\n");
//	exit(sig);
//}

int main(int argc, char *argv[])
{
	pthread_t console_thread;

	//	signal(SIGINT, signal_handler);

	const struct option long_options[] = {
		{"udp", required_argument, NULL, 'u'}, // udp порт для 12-ого гейта.
		{"serial", required_argument, NULL, 'S'}, // serial...

		{"qos", required_argument, NULL,
		 'q'}, // qos отправляемых сообщений. 0 по умолчанию
		{"type", required_argument, NULL,
		 't'}, // qos отправляемых сообщений. 0 по умолчанию

		{"echo", no_argument, NULL,
		 'E'}, //Активирует функцию эха входящих пакетов.
		{"noend", no_argument, NULL,
		 'e'}, //Активирует функцию эха входящих пакетов.
		{"info", no_argument, NULL, 'i'}, //Активирует информацию о вратах.
		{"api", no_argument, NULL, 'a'}, //Активирует информацию о вратах.
		{"noconsole", no_argument, NULL, 'n'}, //Активирует информацию о вратах.
		{"pulse", required_argument, NULL,
		 'p'}, //Активирует информацию о вратах.
		{"debug", no_argument, NULL, 'd'}, //Активирует информацию о вратах.
		{"vdebug", no_argument, NULL, 'v'}, //Активирует информацию о вратах.
		{"gdebug", no_argument, NULL, 'g'}, //Активирует информацию о вратах.
		{NULL, 0, NULL, 0}};

	int long_index = 0;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "uqSEeidvgt", long_options,
							  &long_index)) != -1)
	{
		switch (opt)
		{
		case 'q':
			qos = (uint8_t)atoi(optarg);
			break;
		case 't':
			type = (uint8_t)atoi(optarg);
			break;
		case 'u':
			udpport = (uint16_t)atoi(optarg);
			break;
		case 'S':
			serial_port = (char *)malloc(strlen(optarg) + 1);
			strcpy(serial_port, optarg);
			break;

		case 'E':
			echo = true;
			break;

		case 'e':
			noend = true;
			break;

		case 'i':
			info = true;
			break;

		case 'n':
			noconsole = true;
			break;

		case 'g':
			gdebug = true;
			break;

		case 'p':
			pulse = optarg;
			break;

		case 'a':
			api = true;
			break;

		case 'd':
			crow::enable_diagnostic();
			break;

		case 'v':
			crow::enable_live_diagnostic();
			break;

		case 0:
			break;
		}
	}

	if (crow::create_udpgate(G1_UDPGATE, udpport) == NULL)
	{
		perror("udpgate open");
		exit(-1);
	}

	crow::user_incoming_handler = incoming_handler;
	// crow_transit_handler = transit_handler;

	/*if (!serial_port.empty()) {
		if (crow_create_serialgate(serial_port.c_str(), 115200, 42) == NULL) {
			perror("serialgate open");
			exit(-1);
		}
		//auto ser = new serial::Serial(serial_port, 115200);
		//auto* serial = new owl::io::file(ser->fd());
		//auto* serialgate = new crow::serial_gstuff_gate(ser);
		//crow_link_gate(serialgate, 42);
	}*/

	if (serial_port != NULL)
	{
		if (crow::create_serial_gstuff(serial_port, 115200, 42, gdebug) == NULL)
		{
			perror("serialgate open");
			exit(-1);
		}
	}

	if (optind < argc)
	{
		addrsize = hexer(addr, 128, argv[optind], strlen(argv[optind]));

		if (addrsize < 0)
		{
			printf("Wrong address format\n");
			exit(-1);
		}
	}
	else
	{
		addrsize = 0;
	}

	if (info)
	{
		printf("gates:\n");
		printf("\tgate %d: udp port %d\n", G1_UDPGATE, udpport);

		if (serial_port != 0)
			printf("\tgate %d: serial port %s\n", 42, serial_port);

		// printf("modes:\n");
	}

	if (pulse != std::string())
	{
		if (!noend)
			pulse = pulse + '\n';
		crow::send(addr, (uint8_t)addrsize, pulse.data(),
				   (uint16_t)pulse.size(), type, qos, ackquant);
		crow::onestep();
		exit(0);
	}

	if (!noconsole)
		if (pthread_create(&console_thread, NULL, console_listener, NULL))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}

	crow::spin();
}
