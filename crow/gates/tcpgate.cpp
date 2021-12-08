#include <crow/gates/tcpgate.h>


std::shared_ptr<crow::tcpgate> crow::create_tcpgate_safe(uint8_t id,
        uint16_t port)
{
	int sts;

	auto g = std::make_shared<crow::tcpgate>();
	if ((sts = g->open(port)))
		return g;

	if ((sts = g->bind(id)))
	{
		g->close();
		return g;
	}

	return g;
}

int crow::tcpgate::open(uint16_t port)
{
	server.init();
	int sts = server.bind("0.0.0.0", port);
	if (sts)
		return sts;

	server.nonblock(true);

	return sts;
}

void crow::tcpgate::close()
{
	server.close();

	for (auto& s : sockets)
	{
		s.second.close();
	}
}


void crow::tcpgate::send(crow::packet *) 
{
	nos::println("tcp send");
}

void crow::tcpgate::nblock_onestep() 
{
	nos::println("tcp onestep");
}