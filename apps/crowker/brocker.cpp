#include "brocker.h"
#include <crow/tower.h>
#include <crow/pubsub.h>
#include <nos/print.h>
#include <nos/fprint.h>
#include <igris/util/dstring.h>
#include <sys/uio.h>

bool brocker_info;

std::map<std::string, brocker::theme> brocker::themes;
std::map<std::string, brocker::subscribers::crow> brocker::subscribers::crow::allsubs;
std::map<nos::inet::netaddr, brocker::subscribers::tcp> brocker::subscribers::tcp::allsubs;

void brocker::erase_crow_subscriber(const std::string & addr)
{
	subscribers::crow::allsubs.erase(addr);
}

void brocker::erase_tcp_subscriber(const nos::inet::netaddr & addr)
{
	subscribers::tcp::allsubs.erase(addr);
}

void brocker::publish(const std::string& theme, const std::string& data)
{
	if (brocker_info)
	{
		nos::fprintln("publish: t:{} d:{}", theme, igris::dstring(data));
	}

	try
	{
		auto &thm = themes.at(theme);
		thm.publish(data);
	}
	catch (std::exception ex)
	{
		// owl::println("unres theme");
	}
}

/*

void g3_brocker_subscribe(uint8_t *raddr, size_t rlen, const std::string &theme,
						  uint8_t qos, uint16_t ackquant)
{
	if (brocker_info)
	{
		nos::fprintln("g3_subscribe: t:{} f:{}", theme,
					  igris::hexascii_encode(raddr, rlen));
	}

	if (themes.count(theme) == 0)
	{
		themes[theme] = crow::theme(theme);
	}

	auto &thm = themes[theme];
	thm.subs[std::string((char *)raddr, rlen)] =
		crow::g3_subscriber(raddr, rlen, qos, ackquant);
	// if (ret.second == false)
	//{
	// auto s = crow::g3_subscriber(raddr, rlen, qos, ackquant);
	// thm.subs[s]->qos = qos;
	// thm.subs[s]->ackquant = ackquant;
	//}
}
*/
void brocker::theme::publish(const std::string &data)
{
	for (auto * sub : subs)
	{
		sub->publish(name, data);
	}

	/*struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = name.size();
	subps_d.datsz = data.size();

	iovec vec[4] = {{&subps, sizeof(subps)},
					{&subps_d, sizeof(subps_d)},
					{(void *)name.data(), name.size()},
					{(void *)data.data(), data.size()}};

	for (auto &sub : subs)
	{
		crow::send_v(sub.second.host.data(), sub.second.host.size(), vec, 4,
					 CROW_PUBSUB_PROTOCOL, sub.second.qos, sub.second.ackquant);
	}*/
}

void brocker::subscribers::crow::publish(const std::string & theme, const std::string & data)
{
	/*struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = theme.size();
	subps_d.datsz = data.size();

	iovec vec[4] = {{&subps, sizeof(subps)},
					{&subps_d, sizeof(subps_d)},
					{(void *)theme.data(), theme.size()},
					{(void *)data.data(), data.size()}};

	crow::send_v(addr.data(), addr.size(), vec, 4,
					 CROW_PUBSUB_PROTOCOL, qos, ackquant);*/
	::crow::publish((uint8_t*)addr.data(), addr.size(), theme.c_str(), data, qos, ackquant);
}

void brocker::subscribers::tcp::publish(const std::string & theme, const std::string & data)
{
	std::string str = nos::format("p{:f02>}{}{:f06>}{}", theme.size(), theme, data.size(), data);
	sock->write(str.data(), str.size());
}

brocker::theme * get_theme(const std::string& name) 
{
	if (brocker::themes.count(name)) 
	{
		return &brocker::themes[name];
	}
	else 
	{
		brocker::theme * thm = &brocker::themes[name];
		thm->name = name;
		return thm;
	}
}

void brocker::crow_subscribe(uint8_t*addr, int alen,
                             const std::string& theme,
                             uint8_t qos, uint16_t ackquant)
{
	if (brocker_info)
	{
		nos::fprintln("subscribe(crow): a:{} t:{}", igris::dstring(addr, alen), theme);
	}

	std::string saddr{(char*)addr, (size_t)alen};
	brocker::theme * thm = get_theme(theme);
	auto * sub = brocker::subscribers::crow::get(saddr);
	sub->addr = saddr;
	sub->qos = qos;
	sub->ackquant = ackquant;
	thm->link_subscriber(sub);
}

void brocker::unlink_theme_subscriber(brocker::theme* thm, brocker::subscriber* sub) 
{
	thm->unlink_subscriber(sub);
	if (thm->count_subscribers() == 0) 
	{
		brocker::themes.erase(thm->name);
	} 
}

void brocker::tcp_subscribe(const std::string& theme, nos::inet::tcp_socket * sock) 
{
	if (brocker_info)
	{
		nos::fprintln("subscribe(tcp): a:{}", sock->getaddr());

		brocker::theme * thm = get_theme(theme);
		auto * sub = brocker::subscribers::tcp::get(sock->getaddr());
		sub->sock = sock;
		thm->link_subscriber(sub);
	}
}