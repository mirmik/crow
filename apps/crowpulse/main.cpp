#include <crow/gates/serial_gstuff.h>
#include <crow/gates/udpgate.h>

#include <crow/tower.h>
#include <crow/address.h>
//#include <crow/pubsub/pubsub.h>
//#include <crow/proto/channel.h>
//#include <crow/proto/acceptor.h>

#include <thread>
#include <chrono>

#include <nos/fprint.h>

using namespace std::chrono_literals;

int main(int argc, char** argv) 
{
/*	if (argc != 3) 
	{
		nos::println("usage: crowpulse ADDRESS SLEEPTIME_MS");
		exit(0);
	} 

	auto address = crow::address(argv[1]);
	int sleeptime = atoi(argv[2]);
	
	crow::create_udpgate(12, 0);

	int32_t i = 0; 
	while(1) 
	{
		crow::publish(address, "pulse", nos::format("{}", i++), 0, 100);
		crow::onestep();
		std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
	}*/
}
