#include <crow/tower.h>
#include <crow/channel.h>

#include <nos/trace.h>
#include <nos/print.h>

#include <thread>

void server_incomming(crow::channel* ch, crow::packet* pack) 
{
<<<<<<< HEAD
	(void) ch;

=======
	TRACE();
>>>>>>> f386eb792186b9adccdb59f2a516e580a76a7d05
	igris::buffer data = crow::channel::getdata(pack);
	nos::println(data);
}

crow::channel * create_channel() 
{
	TRACE();
	crow::channel * ch = new crow::channel(server_incomming);
	return ch;
}

int main() 
{
<<<<<<< HEAD
	const struct option long_options[] =
	{
		{"udp", required_argument, NULL, 'u'}, // udp порт для 12-ого гейта.

		{"listen", no_argument, NULL, 'l'},

		{"info", no_argument, NULL, 'i'}, // Выводит информацию о имеющихся гейтах и режимах.
		{"debug", no_argument, NULL, 'd'}, // Включает информацию о событиях башни.
		{"vdebug", no_argument, NULL, 'v'}, // Активирует информацию о времени жизни пакетов.
		{"gdebug", no_argument, NULL, 'g'}, // Активирует информацию о вратах.
		{NULL, 0, NULL, 0}
	};
=======
	crow::diagnostic_enable();
	crow::enable_node_subsystem();
>>>>>>> f386eb792186b9adccdb59f2a516e580a76a7d05

	crow::channel sendch(nullptr);
	crow::link_channel(&sendch, 2);

	crow::create_acceptor(1, &create_channel);

	std::thread thr(crow::spin); thr.detach();

	sendch.handshake(NULL, 0, 1);
	while(sendch.state() != CROW_CHANNEL_CONNECTED);

	sendch.send("HelloWorld", 10);

	while(1);
}