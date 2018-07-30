#include <crow/node.h>
//#include <crow/nodes/test.h>
//#include <crow/nodes/action.h>

#include <crow/channel.h>
#include <crow/channels/test.h>
#include <crow/channels/echo.h>

#include <crow/tower.h>
#include <crow/indexes.h>
#include <crow/gates/udpgate.h>

int main() {
	crow::enable_diagnostic();

	auto tch = crow::create_test_channel(10);
	auto ech = crow::create_echo_channel(11);
	
	//crow::__node_send(0,11,nullptr,0,"HelloWorld",10,crow::QoS(0));
	
	crow::handshake(tch, 11, nullptr, 0);
	crow::__channel_send(tch, "HelloWorld", 10);

	crow::spin();
}