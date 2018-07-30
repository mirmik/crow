#include <crow/tower.h>
#include <crow/pubsub.h>
#include <crow/gates/udpgate.h>
#include <thread>
#include <getopt.h>

crow::udpgate ugate;

int main(int argc, char* argv[]) {
	crow::link_gate(&ugate, G1_UDPGATE);
	ugate.open();

	if (argc != 3) {
		gxx::println("Usage: crow_publish theme data");
		exit(-1);
	}

	if (getenv("CROWKER") == nullptr) {
		gxx::println("Enviroment variable CROWKER doesn't setted");
		exit(-1);
	}

	//gxx::println("brocker: ", getenv("CROWKER"));
	//gxx::println("theme: ", argv[optind]);
	//gxx::println("data: ", argv[optind+1]);

	std::string theme = argv[optind];
	std::string data = argv[optind+1];

	crow::set_publish_host(crow::host(getenv("CROWKER")));

	crow::publish(theme.data(), theme.size(), data.data(), data.size());

	crow::onestep_travel_only();
}

