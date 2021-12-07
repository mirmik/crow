#include <crow/nodes/node_delegate.h>
#include <crow/brocker/crowker.h>
#include <crow/brocker/crowker_api.h>
#include <string>

extern crow::crowker_api crowker_api;

static void themes(crow::node_packet_ptr pack) 
{
	std::string ret;
	auto& themes = crow::crowker::instance()->themes;

	ret.append(nos::format("themes.size: {}\n", themes.size()));
	for (auto& a : themes)
	{
		std::string s = nos::format("name:{} subs:{}\n",
		                            a.first,
		                            a.second.subs.size());
		ret.append(s);
	}

	pack.reply(ret);	
}

static void clients(crow::node_packet_ptr pack) 
{
	std::string ret;
	auto clients = crow::crowker::instance()->clients();

	ret.append(nos::format("clients.size: {}\n", clients.size()));
	for (auto& a : clients)
	{
		std::string s = nos::format("name:{}\n", a->name);
		ret.append(s);
	}

	pack.reply(ret);	
}

static void incoming(crow::node_packet_ptr pack)
{
	if      (pack.message() == "themes") { themes(pack); return; }
	else if (pack.message() == "clients") { clients(pack); return; }

	pack.reply("Unrecognized crowker control command.\n");
}

static void undelivered(crow::node_packet_ptr pack)
{
	(void) pack;
}


static void incoming_beam(crow::node_packet_ptr pack)
{
	crowker_api.client_beam(pack->addr(), pack.sid(), pack.message());
}

crow::node_delegate control(incoming, undelivered);
crow::node_delegate beamsocket(incoming_beam, undelivered);

void init_control_node()
{
	control.bind(CROWKER_CONTROL_BROCKER_NODE_NO);
	beamsocket.bind(CROWKER_BEAMSOCKET_BROCKER_NODE_NO);
}