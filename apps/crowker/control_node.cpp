#include <crow/nodes/node_delegate.h>
#include <crow/brocker/crowker.h>
#include <string>

static void incoming(crow::node_packet_ptr pack)
{
	std::string ret;
	auto& themes = crow::crowker::instance()->themes;

	for (auto& a : themes)
	{
		std::string s = nos::format("name:{} subs:{}\n",
		                            a.first, 
		                            a.second.subs.size());
		ret.append(s);
	}

	pack.reply(ret);
}

static void undelivered(crow::node_packet_ptr pack)
{
	(void) pack;
}

crow::node_delegate control(incoming, undelivered);

/*int hello()
{
	return 42;
}

std::string get_themes()
{
}*/

void init_control_node()
{
	control.bind(2);

	//rpc.add_delegate("hello", igris::make_delegate(hello));
	//rpc.add_delegate("themes", igris::make_delegate(get_themes));
}