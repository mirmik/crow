#include <crow/tower.h>
#include <crow/pubsub.h>
#include <owl/print/stdprint.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

#include <sstream>

#include <owl/trent/trent.h>
#include <owl/trent/gbson.h>
#include <owl/util/hexer.h>

uint8_t crowker_addr[256];
size_t crowker_len;

bool noend = false;
uint8_t qos = 0;
uint8_t acktime = 200;
bool gbson_flag = false;
bool bin_mode = false;
size_t binsize;
std::vector<size_t> binsizes;
std::vector<std::string(*)(void*)> binrestores;
std::vector<std::string> binformat;

std::map<std::string, size_t> visitor_size =
{
	{"flt32", 4},
	{"int32", 4}
};

std::string flt32_restore(void* tgt)
{
	float arg;
	memcpy(&arg, tgt, 4);
	return owl::format("{}", arg);
}

std::string int32_restore(void* tgt)
{
	int32_t arg;
	memcpy(&arg, tgt, 4);
	return owl::format("{}", arg);
}

std::map<std::string, std::string(*)(void* tgt)> visitor_restore =
{
	{"flt32", flt32_restore},
	{"int32", int32_restore}
};

void subscribe_handler(crow::packet* pack)
{
	struct crow_subheader_pubsub * shps = crow::get_subheader_pubsub(pack);
	struct crow_subheader_pubsub_data * shps_d =
	    get_subheader_pubsub_data(pack);

	auto theme = std::string(crow::packet_pubsub_thmptr(pack), shps->thmsz);
	auto data = std::string(crow::packet_pubsub_datptr(pack), shps_d->datsz);

	if (bin_mode) 
	{
		bool wrsize = false;
		if (shps_d->datsz != binsize) 
		{
			wrsize = true;
			owl::println("wrong size");
		}

		uint8_t* ptr = (uint8_t*)crow::packet_pubsub_datptr(pack);
		for (int i = 0; i < binformat.size(); ++i) 
		{
			auto str = binrestores[i](ptr);
			auto fstr = binformat[i]; 
			ptr += binsizes[i];

			owl::fprint("{}:{} ", fstr, str);
		}
		owl::println();

		if (wrsize) {
			owl::println(theme);
			owl_PRINT(shps->thmsz);
			owl_PRINT(shps_d->datsz);
			crow::diagnostic("binmode", pack);
			exit(0);
		}

		fflush(stdout);
		crow::release(pack);
		return;
	}

	if (gbson_flag)
	{
		owl::trent tr;
		owl::gbson::load(tr, data.data(), data.size());
		owl::println(tr);
		fflush(stdout);
		crow::release(pack);
		return;
	}

	owl::print(owl::dstring(data));
	if (!noend) owl::println();
	fflush(stdout);
	
	crow::release(pack);
	return;
}

void undelivered_handler(crow::packet* pack)
{
	dprln("Crowker access error");
	exit(-1);
}

int main(int argc, char* argv[])
{
	const char* crowker = getenv("CROWKER");

	const struct option long_options[] =
	{
		{"crowker", required_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"vdebug", no_argument, NULL, 'v'},
		{"gbson", no_argument, NULL, 'g'},
		{"noend", no_argument, NULL, 'e'}, //Активирует функцию эха входящих пакетов.
		{"bin", required_argument, NULL, 'b'},
		{"qos", required_argument, NULL, 'q'},
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "cdvgq", long_options, &long_index)) != -1)
	{
		switch (opt)
		{
			case 'c': crowker = optarg; break;
			case 'e': noend = true; break;
			case 'd': crow::enable_diagnostic(); break;
			case 'v': crow::enable_live_diagnostic(); break;
			case 'g': gbson_flag = true;
			case 'b': 
				{
					bin_mode = true;
					binformat = owl::split(optarg, ',');
				}
			case 'q': qos = atoi(optarg);
			case 0: break;
		}
	}

	if (crow::create_udpgate(CROW_UDPGATE, 0) == NULL)
	{
		perror("udpgate open");
		exit(-1);
	}

	if (argc - optind != 1)
	{
		owl::println("Usage: crow_publish theme");
		exit(-1);
	}

	if (crowker == nullptr)
	{
		owl::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	if (bin_mode) 
	{
		for (auto& str : binformat) 
		{
			binsize += visitor_size[str];
			binsizes.push_back(visitor_size[str]);
			binrestores.push_back(visitor_restore[str]);
		}
	}

	crowker_len = hexer(crowker_addr, 128, crowker, strlen(crowker));
	crow::set_publish_host(crowker_addr, crowker_len);

	crow::undelivered_handler = undelivered_handler;
	crow::pubsub_handler = subscribe_handler;

	std::string theme = argv[optind];

	crow::subscribe(theme.data(), qos, acktime);
	crow::spin();
}

