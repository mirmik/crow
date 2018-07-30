#include <crow/channel.h>
#include <crow/channels/test.h>
#include <crow/channels/echo.h>

#include <crow/tower.h>
#include <crow/gates/udpgate.h>

int main() {
	crow::enable_diagnostic();

	auto tch = crow::create_test_channel(10);
	auto ech = crow::create_echo_channel(11);

	//crow::host remote(".12.127.0.0.1:5025");

	tch->handshake(crow::host(""), 11);
	tch->send("HelloWorld", 10);

	crow::spin();
}