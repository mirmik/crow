#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

#include <owl/trent/trent.h>
#include <owl/trent/json.h>
#include <owl/trent/gbson.h>

#include <owl/util/numconvert.h>
#include <owl/util/hexer.h>

#include <sstream>

uint8_t crowker_addr[256];
size_t crowker_len;

bool gbson_parse = false;
bool bindata = false;

uint8_t qos = 0;
uint16_t acktime = DEFAULT_ACKQUANT;

std::map<std::string, size_t> visitor_size =
{
	{"flt32", 4},
	{"int32", 4}
};

void flt32_conv(const std::string& str, void* tgt)
{
	*(float*)tgt = (float)atof(str.c_str());

}

void int32_conv(const std::string& str, void* tgt)
{
	*(int32_t*)tgt = atoi(str.c_str());
}

std::map<std::string, void(*)(const std::string& str, void* tgt)> visitor_conv =
{
	{"flt32", flt32_conv},
	{"int32", int32_conv}
};


int main(int argc, char* argv[])
{
	const char* crowker = getenv("CROWKER");

	const struct option long_options[] =
	{
		{"crowker", required_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"vdebug", no_argument, NULL, 'v'},
		{"gbson", no_argument, NULL, 'g'},
		{"bindata", no_argument, NULL, 'b'},
		{"qos", required_argument, NULL, 'q'},
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "cdvgb", long_options, &long_index)) != -1)
	{
		switch (opt)
		{
			case 'c': crowker = optarg; break;
			case 'g': gbson_parse = true; break;
			case 'b': bindata = true; break;
			case 'q': qos = (uint8_t) atoi(optarg); break;
			case 'd': crow::enable_live_diagnostic(); break;
			case 'v': crow::enable_diagnostic(); break;
			case 0: break;
		}
	}

	if (crow::create_udpgate(CROW_UDPGATE, 0) == NULL)
	{
		perror("udpgate open");
		exit(-1);
	}

	if (argc - optind != 2)
	{
		owl::println("Usage: crow_publish theme data");
		exit(-1);
	}

	if (crowker == nullptr)
	{
		owl::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	std::string theme = argv[optind];
	std::string data = argv[optind + 1];

	owl_PRINT(theme);
	owl_PRINT(data);

	crowker_len = hexer(crowker_addr, 128, crowker, strlen(crowker));
	crow::set_publish_host(crowker_addr, crowker_len);

	if (gbson_parse)
	{
		char buf[256];
		std::stringstream istrm(data);
		owl::trent tr = owl::json::parse(istrm).unwrap();
		size_t len = owl::gbson::dump(tr, buf, 256);

		crow::publish_buffer(theme.data(), buf, (uint16_t) len, qos, acktime);
	}
	else if (bindata)
	{
		dprln("bindata mode");

		auto sv = owl::split(argv[optind + 1], ',');
		owl::println(sv);

		using ptype = std::pair<std::string, std::string>;
		std::vector<ptype> vec;

		for (auto s : sv)
		{
			auto p = owl::split(s, ':');
			vec.emplace_back(p[0], p[1]);
		}

		//owl::println(vec);

		size_t sz = 0;

		for (auto s : vec) { sz += visitor_size[s.first]; };

		uint8_t* block = (uint8_t*)malloc(sz);
		uint8_t* ptr = block;

		for (auto s : vec)
		{
			owl::println(s.first);
			visitor_conv[s.first](s.second, ptr);
			ptr += visitor_size[s.first];
		}

		owl::print_dump(block, sz);

		crow::publish_buffer(theme.data(), block, (uint16_t)sz, qos, acktime);
	}
	else
	{
		crow::publish(theme.data(), data.data(), qos, acktime);
	}
	crow::onestep_travel_only();
}

