#include <crow/nodes/node_delegate.h>
#include <crow/brocker/crowker.h>
#include <crow/brocker/crowker_api.h>
#include <igris/shell/rshell_executor.h>
#include <string>

extern crow::crowker_api crowker_api;

static int themes(int, char **, char * ans, int ansmax) 
{	
	int sz = 0;
	nos::buffer_writer writer(ans, ansmax);
	std::string ret;

	auto& themes = crow::crowker::instance()->themes;
	if (themes.size() == 0)
		return nos::println_to(writer, "No one theme here yet.");

	for (auto& a : themes)
	{
		std::string s = nos::format("name:{} subs:{}\n",
		                            a.first,
		                            a.second.subs.size());
		sz += nos::print_to(writer, s);
	}

	return sz;
}

static int clients(int, char **, char * ans, int ansmax) 
{
	int sz = 0;
	nos::buffer_writer writer(ans, ansmax);
	std::string ret;

	auto clients = crow::crowker::instance()->clients();
	if (clients.size() == 0)
		return nos::println_to(writer, "No one theme here yet.");

	for (auto& a : clients)
	{
		std::string s = nos::format("name:{}\n", a->name);
		sz += nos::print_to(writer, s);
	}

	return sz;
}

rshell_command cmds[] = {
	{"clients", clients, "crowker clients"},
	{"themes", themes, "crowker themes"},
	{NULL, NULL, 0}
};
igris::rshell_executor_onelevel executor(cmds);

static void incoming(crow::node_packet_ptr pack)
{
	char buf[128];
	executor.execute(pack.message().data(), pack.message().size(), buf, 128);	
	pack.reply({buf, strlen(buf)});
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