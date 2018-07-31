#include <crow/tower.h>
#include <crow/pubsub.h>
#include <gxx/print/stdprint.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

crow::udpgate ugate;

void subscribe_handler(crow::packet* pack) {
	auto thm = crow::pubsub_theme(pack);
	auto dat = crow::pubsub_data(pack);

	gxx::println(gxx::dstring(dat));
}

void undelivered_handler(crow::packet* pack) {
	gxx::println("Crowker access error");
	exit(-1);
}

int main(int argc, char* argv[]) {
	const char* crowker = getenv("CROWKER");

	const struct option long_options[] = {
		{"crowker", required_argument, NULL, 'c'},
		{"debug", no_argument, NULL, 'd'},
		{NULL,0,NULL,0}
	};

    int long_index =0;
	int opt= 0;
	while ((opt = getopt_long(argc, argv, "c", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'c': crowker = optarg; break;
			case 'd': crow::enable_diagnostic(); break;
			case 0: break;
		}
	}

	crow::undelivered_handler = undelivered_handler;

	crow::link_gate(&ugate, G1_UDPGATE);
	ugate.open();

	if (argc - optind != 1) {
		gxx::println("Usage: crow_publish theme");
		exit(-1);
	}

	if (crowker == nullptr) {
		gxx::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	//gxx::println("brocker:", crowker);
	//gxx::println("theme:", argv[optind]);

	std::string theme = argv[optind];

	crow::set_publish_host(crow::host(crowker));
	crow::set_publish_qos(crow::QoS(2));

	crow::subscribe_handler = subscribe_handler;

	crow::subscribe(theme.data(), theme.size());
	crow::spin();
}

