#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

crow::udpgate ugate;

void subscribe_handler(crow::packet* pack) {
	auto thm = crow::pubsub_theme(pack);
	auto dat = crow::pubsub_data(pack);

	gxx::println(dat);
}

int main(int argc, char* argv[]) {
	//crow::enable_diagnostic();

	crow::link_gate(&ugate, G1_UDPGATE);
	ugate.open();

	if (argc != 2) {
		gxx::println("Usage: crow_publish theme");
		exit(-1);
	}

	if (getenv("CROWKER") == nullptr) {
		gxx::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	//gxx::println("brocker: ", getenv("CROWKER"));
	//gxx::println("theme: ", argv[optind]);

	std::string theme = argv[optind];

	crow::set_publish_host(crow::host(getenv("CROWKER")));
	crow::subscribe_handler = subscribe_handler;

	crow::subscribe(theme.data(), theme.size());
	crow::spin();
}

