#include <crow/nodes/node_delegate.h>
#include <crow/brocker/crowker.h>
#include <crow/brocker/crowker_api.h>
#include <igris/shell/rshell_executor.h>
#include <string>

#include <nos/shell/argv.h>
#include <nos/shell/executor.h>

extern crow::crowker_api crowker_api;

static int themes(const nos::argv&, nos::ostream& os) 
{	
	auto& themes = crow::crowker::instance()->themes;
	if (themes.size() == 0) 
	{
		os.println("No one theme here yet.");
		return 0;
	}

	for (auto& a : themes)
	{
		std::string s = nos::format("name:{} subs:{}\n",
		                            a.first,
		                            a.second.subs.size());
		os.print(s);
	}
	return 0;
}

static int clients(const nos::argv&, nos::ostream& os) 
{
	auto clients = crow::crowker::instance()->clients();
	if (clients.size() == 0) 
	{
		os.println("No one theme here yet.");
		return 0;
	}

	for (auto& a : clients)
	{
		std::string s = nos::format("name:{}\n", a->name);
		os.print(s);
	}
	return 0;
}

nos::executor executor({
	nos::command{"clients", "crowker clients", clients},
	{"themes", "crowker themes", themes}
});

static void incoming(crow::node_packet_ptr pack)
{
	nos::string_buffer answer;
	executor.execute(nos::tokens(pack.message().data()), answer);	
	pack.reply({answer.str().data(), answer.str().size()});
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