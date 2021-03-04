#include "brocker.h"
#include <crow/tower.h>
#include <crow/pubsub/pubsub.h>
#include <nos/print.h>
#include <nos/fprint.h>
#include <igris/util/dstring.h>
#include <sys/uio.h>

#include <chrono>

bool brocker_info = false;
bool log_publish = false;

std::map<std::string, brocker::theme> brocker::themes;
std::map<std::string, brocker::subscribers::crow> brocker::subscribers::crow::allsubs;
std::map<nos::inet::netaddr, brocker::subscribers::tcp> brocker::subscribers::tcp::allsubs;

int64_t eval_timestamp() 
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return millis;
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

void brocker::erase_crow_subscriber(const std::string & addr)
{
	subscribers::crow::allsubs.erase(addr);
}

void brocker::erase_tcp_subscriber(const nos::inet::netaddr & addr)
{
	subscribers::tcp::allsubs.erase(addr);
}

void brocker::publish(std::shared_ptr<std::string> theme, std::shared_ptr<std::string> data)
{
	int subs_count;

	auto * thm = get_theme(*theme);
	subs_count = thm->count_subscribers();

	thm->publish(data);
	
	if (brocker_info || log_publish)
	{
		nos::fprintln("publish: t:{} s:{} d:{}",
		              *theme, subs_count, igris::dstring(*data));
	}

}

void brocker::theme::publish(std::shared_ptr<std::string> data)
{
	for (auto * sub : subs)
	{
		sub->publish(name, {data->data(), data->size()}, sub->thms[this].opts);
	}

	timestamp_publish = timestamp_activity = eval_timestamp();
}

void brocker::subscribers::crow::publish(const std::string & theme, const std::string & data, options * opts)
{
	crow_options * copts = static_cast<crow_options*>(opts);

	::crow::publish(
		{addr.data(), addr.size()}, 
		theme.c_str(), 
		{data.data(), data.size()}, 
		copts->qos, 
		copts->ackquant);
}

void brocker::subscribers::tcp::publish(const std::string & theme, const std::string & data, options * opts)
{
	assert(opts == nullptr);
	std::string str = nos::format("p{:f02>}{}{:f06>}{}", theme.size(), theme, data.size(), data);
	sock->write(str.data(), str.size());
}

void brocker::crow_subscribe(uint8_t*addr, int alen,
                             const std::string& theme,
                             uint8_t qos, uint16_t ackquant)
{
	std::string saddr{(char*)addr, (size_t)alen};
	brocker::theme * thm = get_theme(theme);
	thm->timestamp_activity = eval_timestamp();

	auto * sub = brocker::subscribers::crow::get(saddr);
	
	// TODO: Перенести. Незачем перезаписывать адресс каждый раз.
	sub->addr = saddr;

	if (!thm->has_subscriber(sub))
	{
		thm->link_subscriber(sub);
		brocker::options*& opts = sub->thms[thm].opts;
		opts = new brocker::subscribers::crow_options{qos, ackquant};

		if (brocker_info)
			nos::fprintln("new subscribe(crow): a:{} q:{} c:{} t:{}", igris::dstring(addr, alen), qos, ackquant, theme);
	} else 
	{
		brocker::subscribers::crow_options * opts = 
			static_cast<brocker::subscribers::crow_options *>
				(sub->thms[thm].opts);
		if (opts->qos != qos  || opts->ackquant != ackquant) 
		{
			opts->qos = qos;
			opts->ackquant = ackquant;
			nos::fprintln("change subscribe(crow): a:{} q:{} c:{} t:{}", igris::dstring(addr, alen), qos, ackquant, theme);		
		}
	}
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

	brocker::theme * thm = get_theme(theme);
	auto * sub = brocker::subscribers::tcp::get(sock->getaddr());
	sub->sock = sock;

	if (!thm->has_subscriber(sub))
	{
		thm->link_subscriber(sub);

		if (brocker_info)
			nos::fprintln("subscribe(tcp): a:{}", sock->getaddr());
	}
}