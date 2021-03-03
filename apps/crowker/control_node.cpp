#include <crow/proto/rpc.h>
#include <crow/brocker/crowker.h>
#include <string>

crow::rpc_node rpc;

int hello() 
{
	return 42;
}

std::string get_themes() 
{
	std::string ret;

	auto& themes = crow::crowker::instance()->themes;

	for (auto& a : themes) 
	{
		std::string s = nos::format("name:{} subs:{} lasta:{} lastp:{}\n", 
			a.first, a.second.subs.size(), a.second.timestamp_activity, a.second.timestamp_publish);
		ret.append(s);
	}

	return ret;
}

void init_control_node() 
{
	rpc.bind();

	rpc.add_delegate("hello", igris::make_delegate(hello));
	rpc.add_delegate("themes", igris::make_delegate(get_themes));
}