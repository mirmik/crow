#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

#include <gxx/trent/trent.h>
#include <gxx/trent/json.h>
#include <gxx/trent/gbson.h>

#include <sstream>

crow::udpgate ugate;
bool gbson_parse = false;

int main(int argc, char* argv[]) {
	crow::link_gate(&ugate, G1_UDPGATE);
	ugate.open();

	const char* crowker = getenv("CROWKER");

	const struct option long_options[] = {
		{"crowker", required_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{"gbson", no_argument, NULL, 'g'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "c", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'c': crowker = optarg; break;
			case 'g': gbson_parse = true;
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
	} else {
		crow::publish(theme.data(), theme.size(), data.data(), data.size());
	}
	crow::onestep_travel_only();
}

