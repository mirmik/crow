#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

#include <gxx/trent/trent.h>
#include <gxx/trent/json.h>
#include <gxx/trent/gbson.h>

#include <gxx/util/numconvert.h>

#include <sstream>

crow::udpgate ugate;
bool gbson_parse = false;
bool bindata = false;

std::map<std::string, size_t> visitor_size = {
	{"flt32", 4},
	{"int32", 4}
};

void flt32_conv(const std::string& str, void* tgt) {
	*(float*)tgt = atof(str.c_str());

}

void int32_conv(const std::string& str, void* tgt) {
	*(int32_t*)tgt = atoi(str.c_str());
}

std::map<std::string, void(*)(const std::string& str, void* tgt)> visitor_conv = {
	{"flt32", flt32_conv},
	{"int32", int32_conv}
};


int main(int argc, char* argv[]) {
	crow::link_gate(&ugate, G1_UDPGATE);
	ugate.open();

	const char* crowker = getenv("CROWKER");

	const struct option long_options[] = {
		{"crowker", required_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"gbson", no_argument, NULL, 'g'},
		{"bindata", no_argument, NULL, 'b'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "cdgb", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'c': crowker = optarg; break;
			case 'g': gbson_parse = true;
			case 'b': bindata = true;
			case 'd': crow::enable_diagnostic(); break;
			case 0: break;
		}
	}

	if (argc - optind != 2) {
		gxx::println("Usage: crow_publish theme data");
		exit(-1);
	}

	if (crowker == nullptr) {
		gxx::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	std::string theme = argv[optind];
	std::string data = argv[optind+1];

	crow::set_publish_host(crow::host(crowker));
	
	if (gbson_parse) {
		char buf[256];
		std::stringstream istrm(data);
		gxx::trent tr = gxx::json::parse(istrm).unwrap();
		int len = gxx::gbson::dump(tr, buf, 256);

		crow::publish(theme.data(), theme.size(), buf, len);
	} else 
	if (bindata) {
		//gxx::println(argv[optind+1]);

		auto sv = gxx::split(argv[optind+1], ',');
		//gxx::println(sv);

		using ptype = std::pair<std::string, std::string>;
		std::vector<ptype> vec;

		for (auto s: sv) { 
			auto p = gxx::split(s, ':');
			vec.emplace_back(p[0], p[1]);
		}

		//gxx::println(vec);

		size_t sz = 0;

		for (auto s : vec) { sz += visitor_size[s.first]; };

		uint8_t* block = (uint8_t*)malloc(sz);
		uint8_t* ptr = block;

		for (auto s : vec) {
			gxx::println(s.first);
			visitor_conv[s.first](s.second, ptr);
			ptr += visitor_size[s.first];
		}

		gxx::print_dump(block, sz);

		crow::publish(theme.data(), theme.size(), block, sz);
	} else {
		crow::publish(theme.data(), theme.size(), data.data(), data.size());
	}
	crow::onestep_travel_only();
}

