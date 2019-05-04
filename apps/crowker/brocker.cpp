#include "brocker.h"
#include <crow/pubsub.h>
#include <nos/print.h>
#include <nos/fprint.h>
#include <igris/util/dstring.h>
#include <sys/uio.h>

std::unordered_map<std::string, crow::theme> themes;
bool brocker_info = false;

void brocker_publish(const std::string &theme, const std::string &data)
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

void crow::theme::publish(const std::string &data)
{
	struct crow_subheader_pubsub subps;
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
					 G1_G3TYPE, sub.second.qos, sub.second.ackquant);
	}
}
