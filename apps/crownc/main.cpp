#include <crow/tower.h>
#include <crow/channel.h>

#include <nos/trace.h>
#include <nos/print.h>

#include <thread>

void server_incomming(crow::channel* ch, crow::packet* pack) 
{
	TRACE();
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
	crow::diagnostic_enable();
	crow::enable_node_subsystem();

	crow::channel sendch(nullptr);
	crow::link_channel(&sendch, 2);

	crow::create_acceptor(1, &create_channel);

	std::thread thr(crow::spin); thr.detach();

	sendch.handshake(NULL, 0, 1);
	while(sendch.state() != CROW_CHANNEL_CONNECTED);

	sendch.send("HelloWorld", 10);

	while(1);
}