#include <crow/tower.h>
#include <crow/channel.h>

#include <nos/print.h>

void server_incomming(crow::channel* ch, crow::packet* pack) 
{
	igris::buffer data = crow::channel::getdata(pack);
	nos::println(data);
}

crow::channel * create_channel() 
{
	crow::channel * ch = new crow::channel(server_incomming);
	return ch;
}

int main() 
{
	crow::channel sendch(nullptr);
	crow::link_channel(&sendch, 2);

	crow::create_acceptor(1, &create_channel);



	crow::spin();
}