#include <crow/gates/serial_gstuff.h>
#include <crow/gates/udpgate.h>

#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/hexer.h>

#include <igris/util/string.h>
#include <igris/util/dstring.h>
#include <igris/util/bug.h>

#include <nos/fprint.h>

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
#include <map>

#include "binin.h"
#include "binout.h"

#include <iostream>

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

bool bin_output_mode = false;
bool bin_input_mode = false;

std::string binout_fmt;
std::string binin_fmt;

std::string pulse;
std::string theme;

enum class protoopt_e
{
	PROTOOPT_BASIC,
	PROTOOPT_PUBLISH
};

enum class input_format
{
	INPUT_RAW,
	INPUT_RAW_ENDLINE,
	INPUT_BINARY
};

enum class output_format
{
	OUTPUT_RAW,
	OUTPUT_DSTRING,
	OUTPUT_BINARY
};

protoopt_e protoopt = protoopt_e::PROTOOPT_BASIC;
input_format informat = input_format::INPUT_RAW_ENDLINE;
output_format outformat = output_format::OUTPUT_RAW;

std::string informat_tostr()
{
	switch (informat)
	{
		case input_format::INPUT_RAW: return "INPUT_RAW";
		case input_format::INPUT_RAW_ENDLINE: return "INPUT_RAW_ENDLINE";
		case input_format::INPUT_BINARY: return "INPUT_BINARY";
	}
	return std::string();
}

std::string outformat_tostr()
{
	switch (outformat)
	{
		case output_format::OUTPUT_RAW: return "OUTPUT_RAW";
		case output_format::OUTPUT_DSTRING: return "OUTPUT_DSTRING";
		case output_format::OUTPUT_BINARY: return "OUTPUT_BINARY";
	}
	return std::string();
}

void output_do(igris::buffer data, crow::packet* pack)
{
	if (api)
	{
		if (data == "exit\n")
			exit(0);
	}

	switch (outformat)
	{
		case output_format::OUTPUT_RAW:
			printf("%.*s", (int)data.size(), data.data());
			fflush(stdout);
			break;

		case output_format::OUTPUT_DSTRING:
			// Вывод в stdout информацию пакета.
			char buf[10000];
			bytes_to_dstring(buf, data.data(), data.size());
			printf("%s\n", buf);
			fflush(stdout);
			break;

		case output_format::OUTPUT_BINARY:
			output_binary(data, pack);
			break;

		default:
			BUG();
	}
}

std::string input_do(const std::string& data)
{
	std::string message;

	switch (informat)
	{
		case input_format::INPUT_RAW_ENDLINE:
			message = std::string(data.data(), data.size());
			message += '\n';
			return message;

		case input_format::INPUT_RAW:
			message = std::string(data.data(), data.size());
			return message;

		case input_format::INPUT_BINARY:
			message = input_binary(std::string(data.data(), data.size()));
			return message;

		default:
			BUG();
	}
}

void send_do(const std::string message)
{
	switch (protoopt)
	{
		case protoopt_e::PROTOOPT_BASIC:
			crow::send(addr, (uint8_t)addrsize,
			           message.data(), message.size(), type,
			           qos, ackquant);
			break;
		case protoopt_e::PROTOOPT_PUBLISH:
			crow::publish(addr, (uint8_t)addrsize, theme.c_str(),
			              message.data(), message.size(),
			              qos, ackquant);
			break;
	}
}

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

	output_do(pack->rawdata(), pack);

	crow::release(pack);
}

void *console_listener(void *arg)
{
	(void)arg;

	std::string input;

	while (1)
	{
		if (std::cin.peek() == std::char_traits<char>::eof()) 
		{
			exit(0);
		}

		std::getline(std::cin, input);

		std::string message = input_do(input);
		send_do(message);
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

		{"binin", required_argument, NULL, 'b'}, // Форматирует вход согласно бинарного шаблона.
		{"binout", required_argument, NULL, 'B'}, // Форматирует вывод согласно бинарного шаблона.
		{"rawout", no_argument, NULL, 'r'},
		{"dbgout", no_argument, NULL, 'j'},

		{"publish", required_argument, NULL, 'P'},

		{"info", no_argument, NULL, 'i'}, // Выводит информацию о имеющихся гейтах и режимах.
		{"debug", no_argument, NULL, 'd'}, // Включает информацию о событиях башни.
		{"vdebug", no_argument, NULL, 'v'}, // Активирует информацию о времени жизни пакетов.
		{"gdebug", no_argument, NULL, 'g'}, // Активирует информацию о вратах.
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;

	while ((opt = getopt_long(argc, argv, "uqSEeidvgtB", long_options,
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

			case 'r':
				outformat = output_format::OUTPUT_RAW;
				break;

			case 'j':
				outformat = output_format::OUTPUT_DSTRING;
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

			case 'b':
				binin_fmt = optarg;
				bin_input_mode = true;
				break;

			case 'B':
				//binary_mode_prepare(optarg);
				binout_fmt = optarg;
				bin_output_mode = true;
				break;

			case 'P':
				theme = optarg;
				protoopt = protoopt_e::PROTOOPT_PUBLISH;
				break;

			case '?':
				exit(-1);
				break;

			case 0:
				BUG();
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

	if (bin_output_mode == true)
	{
		binout_mode_prepare(binout_fmt);
		outformat = output_format::OUTPUT_BINARY;
	}

	if (bin_input_mode == true)
	{
		binin_mode_prepare(binin_fmt);
		informat = input_format::INPUT_BINARY;
	}

	if (noend)
	{
		informat = input_format::INPUT_RAW;
	}

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

		nos::println("informat:", informat_tostr());
		nos::println("outformat:", outformat_tostr());
	}

// Ветка обработки pulse мода.
	if (pulse != "")
	{
		std::string message = input_do(pulse);

		send_do(message);

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
