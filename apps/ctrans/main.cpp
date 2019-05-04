#include <crow/gates/serial_gstuff.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <crow/hexer.h>
#include <igris/util/dstring.h>
#include <igris/util/bug.h>

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

enum class output_format
{
	OUTPUT_RAW,
	OUTPUT_DSTRING,
};

output_format outformat = output_format::OUTPUT_RAW;

void incoming_handler(crow::packet *pack)
{
	if (echo)
	{
		// Переотослать пакет точно повторяющий входящий.
		crow::send(pack->addrptr(), pack->header.alen, pack->dataptr(),
		           pack->datasize(), pack->header.f.type, pack->header.qos,
		           pack->header.ackquant);
	}

	if (api)
	{
		// Запуск встроенных функций.
		char *dp = pack->dataptr();
		size_t ds = pack->datasize();

		if (strncmp(dp, "exit\n", ds) == 0)
		{
			raise(SIGINT);
		}
	}


	switch (outformat)
	{
		case output_format::OUTPUT_RAW:
			printf("%*s", pack->datasize(), pack->dataptr());
			fflush(stdout);
			break;

		case output_format::OUTPUT_DSTRING: 
			// Вывод в stdout информацию пакета.
			char buf[10000];
			bytes_to_dstring(buf, pack->dataptr(), pack->datasize());
			printf("%s\n", buf);
			fflush(stdout);
			break;

		default:
			BUG();
	}

	crow::release(pack);
}

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

int main(int argc, char *argv[])
{
	pthread_t console_thread;

	const struct option long_options[] =
	{
		{"udp", required_argument, NULL, 'u'}, // udp порт для 12-ого гейта.
		{"serial", required_argument, NULL, 'S'}, // serial...

		{"qos", required_argument, NULL, 'q'}, // qos отправляемых сообщений. 0 по умолчанию
		{"type", required_argument, NULL, 't'}, // метка типа отправляемых сообщений
		{"ackquant", required_argument, NULL, 'a'}, // установка кванта ack

		{"noend", no_argument, NULL, 'e'}, // Блокирует добавление символа конца строки.
		{"echo", no_argument, NULL, 'E'}, // Активирует функцию эха входящих пакетов.
		{"api", no_argument, NULL, 'a'}, // Активирует удалённое управление.
		{"noconsole", no_argument, NULL, 'n'}, // Отключает создание консоли.
		{"pulse", required_argument, NULL, 'p'}, // Отключает программу по первой транзакции.

		{"info", no_argument, NULL, 'i'}, // Выводит информацию о имеющихся гейтах.
		{"debug", no_argument, NULL, 'd'}, //Активирует информацию о вратах.
		{"vdebug", no_argument, NULL, 'v'}, //Активирует информацию о вратах.
		{"gdebug", no_argument, NULL, 'g'}, //Активирует информацию о вратах.
		{NULL, 0, NULL, 0}
	};

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

	if (serial_port != NULL)
	{
		if (crow::create_serial_gstuff(serial_port, 115200, 42, gdebug) == NULL)
		{
			perror("serialgate open");
			exit(-1);
		}
	}

// Переопределение стандартного обработчика (Для возможности перехвата и api)
	crow::user_incoming_handler = incoming_handler;


// Определение целевого адресса
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

// Вывод информации о созданных вратах.
	if (info)
	{
		printf("gates:\n");
		printf("\tgate %d: udp port %d\n", G1_UDPGATE, udpport);

		if (serial_port != 0)
			printf("\tgate %d: serial port %s\n", 42, serial_port);
	}

// Ветка обработки pulse мода.
	if (pulse != std::string())
	{
		if (!noend)
			pulse = pulse + '\n';

		crow::send(addr, (uint8_t)addrsize, pulse.data(),
		           (uint16_t)pulse.size(), type, qos, ackquant);
		crow::onestep();
		exit(0);
	}

// Создание консольного ввода, если необходимо.
	if (!noconsole)
	{
		if (pthread_create(&console_thread, NULL, console_listener, NULL))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}

	crow::spin();
}
