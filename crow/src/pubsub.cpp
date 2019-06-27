#include <crow/hexer.h>
#include <crow/pubsub.h>

#include <igris/sync/syslock.h>

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

void crow::publish(
    const uint8_t * raddr, uint8_t rlen,
    const char *theme, const void *data, uint16_t dlen,
    uint8_t qos, uint16_t acktime)
{
	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_data subps_d;

	subps.type = PUBLISH;
	subps.thmsz = (uint8_t)strlen(theme);
	subps_d.datsz = dlen;

	struct iovec iov[4] =
	{
		{&subps, sizeof(subps)},
		{&subps_d, sizeof(subps_d)},
		{(void *)theme, subps.thmsz},
		{(void *)data, subps_d.datsz},
	};

	crow::send_v(raddr, rlen, iov, 4, CROW_PUBSUB_PROTOCOL,
	             qos, acktime);
}


void crow::publish(const uint8_t * raddr, uint8_t rlen,
                   const char *theme, const char *data, uint8_t qos,
                   uint16_t acktime)
{
	crow::publish(raddr, rlen, theme, data, (uint16_t)strlen(data), qos, acktime);
}

void crow::publish(const uint8_t * raddr, uint8_t rlen,
                   const char *theme, const std::string& data, uint8_t qos,
                   uint16_t acktime)
{
	crow::publish(raddr, rlen, theme, data.data(), data.size(), qos, acktime);
}

void crow::subscribe(
    const uint8_t * raddr, uint8_t rlen,
    const char *theme, uint8_t qos, uint16_t acktime,
    uint8_t rqos, uint16_t racktime)
{
	size_t thmsz = strlen(theme);

	struct crow_subheader_pubsub subps;
	struct crow_subheader_pubsub_control subps_c;

	subps.type = SUBSCRIBE;
	subps.thmsz = (uint8_t)thmsz;
	subps_c.qos = rqos;
	subps_c.ackquant = racktime;

	struct iovec iov[3] =
	{
		{&subps, sizeof(subps)},
		{&subps_c, sizeof(subps_c)},
		{(void *)theme, thmsz},
	};

	crow::send_v(raddr, rlen, iov, 3, CROW_PUBSUB_PROTOCOL, qos,
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

void crow::publish(
    const std::vector<uint8_t> & addr,
    const std::string & theme,
    const std::string & data,
    uint8_t qos,
    uint16_t acktime)
{
	publish(
	    (const uint8_t*)addr.data(), addr.size(),
	    theme.c_str(),
	    data.data(), data.size(),
	    qos, acktime);
}

void crow::subscribe(
    const std::vector<uint8_t> & addr,
    const std::string & theme,
    uint8_t qos, uint16_t acktime,
    uint8_t rqos, uint16_t racktime)
{
	subscribe(
	    (const uint8_t*)addr.data(), addr.size(),
	    theme.c_str(),
	    qos, acktime,
	    rqos, racktime);
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