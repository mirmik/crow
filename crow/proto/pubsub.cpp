#include <crow/proto/pubsub.h>
#include <igris/sync/syslock.h>

#include <crow/hexer.h>

crow::pubsub_protocol_cls crow::pubsub_protocol;

void crow::pubsub_protocol_cls::incoming(crow::packet * pack)
{
	crow::subscriber * sub;
	igris::buffer theme = crow::pubsub::get_theme(pack);

	if (incoming_handler)
	{
		incoming_handler(pack);
	}
	else
	{
		dlist_for_each_entry(sub, &themes, lnk)
		{
			if (theme == sub->theme)
			{
				sub->dlg(pack);
				return;
			}
		}
		crow::release(pack);
	}
}

crow::packet* crow::make_publish_packet(
    const uint8_t * raddr, uint8_t rlen,
    const char *theme,
    const void *data, uint16_t dlen)
{
	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = (uint8_t)strlen(theme);
	subps_d.datsz = dlen;

	const igris::buffer iov[4] =
	{
		{&subps, sizeof(subps)},
		{&subps_d, sizeof(subps_d)},
		{(void *)theme, subps.thmsz},
		{(void *)data, subps_d.datsz},
	};

	auto pack = crow::make_packet_v(raddr, rlen, iov, 4);
	if (!pack)
		return nullptr;

	pack->type(CROW_PUBSUB_PROTOCOL);

	return pack;
}

void crow::publish(
    const crow::hostaddr & addr, 
    const std::string_view theme, 
    const igris::buffer data,
    uint8_t qos, uint16_t acktime)
{
	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = theme.size();
	subps_d.datsz = data.size();

	const igris::buffer iov[4] =
	{
		{&subps, sizeof(subps)},
		{&subps_d, sizeof(subps_d)},
		{theme.data(), subps.thmsz},
		data,
	};

	crow::send_v(addr, iov, 4, CROW_PUBSUB_PROTOCOL,
	             qos, acktime);
}

void crow::subscribe(
    const crow::hostaddr & addr, 
    const std::string_view theme, 
    uint8_t qos, uint16_t acktime,
    uint8_t rqos, uint16_t racktime)
{
	size_t thmsz = theme.size();

	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_control subps_c;

	subps.type = SUBSCRIBE;
	subps.thmsz = (uint8_t)thmsz;
	subps_c.qos = rqos;
	subps_c.ackquant = racktime;

	const igris::buffer iov[3] =
	{
		{&subps, sizeof(subps)},
		{&subps_c, sizeof(subps_c)},
		theme,
	};

	crow::send_v(addr, iov, 3, CROW_PUBSUB_PROTOCOL, qos,
	             acktime);
}

std::string crow::envcrowker()
{
	uint8_t buf[128];
	const char *envcr = getenv("CROWKER");
	auto ss = hexer_s(buf, 128, envcr);
	return std::string((char *)buf, ss);
}

std::string crow::environment_crowker()
{
	const char *envcr = getenv("CROWKER");
	return std::string(envcr);
}

void crow::pubsub_protocol_cls::resubscribe_all()
{
	crow::subscriber * sub;

	system_lock();
	dlist_for_each_entry(sub, &themes, lnk)
	{
		sub->resubscribe();
	}
	system_unlock();
}