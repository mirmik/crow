#include <crow/gates/udpgate.h>
#include <crow/pubsub.h>
#include <crow/tower.h>
#include <crow/hexer.h>
#include <crow/alive.h>

#include <crow/netkeep.h>

//#include <thread>
#include <getopt.h>
#include <stdbool.h>

#include <stdio.h>
#include <string>
#include <thread>

#include "brocker.h"

#include <nos/print.h>
#include <nos/fprint.h>

std::string handshake;
std::string netname;

int udpport = -1;
bool quite = false;

void incoming_pubsub_packet(struct crow::packet *pack)
{
	struct crow_subheader_pubsub *shps = crow::get_subheader_pubsub(pack);

	switch (shps->type)
	{
		case PUBLISH:
		{
			struct crow_subheader_pubsub_data *shps_d =
			    get_subheader_pubsub_data(pack);

			auto theme = std::string(crow::packet_pubsub_thmptr(pack), shps->thmsz);
			auto data =
			    std::string(crow::packet_pubsub_datptr(pack), shps_d->datsz);

			brocker_publish(theme, data);
		}
		break;
		case SUBSCRIBE:
		{
			auto shps_c = get_subheader_pubsub_control(pack);
			std::string theme(pack->dataptr() + sizeof(crow_subheader_pubsub) +
			                  sizeof(crow_subheader_pubsub_control),
			                  shps->thmsz);

			g3_brocker_subscribe(pack->addrptr(), pack->addrsize(), theme,
			                     shps_c->qos, shps_c->ackquant);
		}
		break;
		default:
		{
			printf("unresolved pubsub frame type %d", (uint8_t)shps->type);
		}
		break;
	}
	crow::release(pack);
}

void undelivered_handler(struct crow::packet *pack)
{
	if (pack->header.f.type == CROW_PUBSUB_PROTOCOL)
	{
		auto shps = crow::get_subheader_pubsub(pack);

		if (shps->type == PUBLISH)
		{
			std::string theme(pack->dataptr() + sizeof(crow_subheader_pubsub) +
			                  sizeof(crow_subheader_pubsub_data),
			                  shps->thmsz);
			auto &thm = themes[theme];
			thm.subs.erase(
			    std::string((char *)pack->addrptr(), pack->header.alen));
			if (brocker_info)
				nos::fprintln(
				    "g3_refuse: t:{}, r:{}", theme,
				    igris::hexascii_encode(pack->addrptr(), pack->header.alen));
		}
	}

	crow::release(pack);
}

void alivespam()
{
	uint8_t raddr[128];
	uint8_t rlen;

	rlen = hexer_s(raddr, 128, handshake.c_str());

	while (1)
	{
		crow::send_alive(raddr, rlen,
		                 CROW_ALIVE_HANDSHAKE, CROW_TOWER_TYPE_CROWKER,
		                 0, 200);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

}

void netserve()
{
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		crow::netkeep_serve();
	}
}

int main(int argc, char *argv[])
{
	crow::pubsub_handler = incoming_pubsub_packet;
	crow::undelivered_handler = undelivered_handler;

	crow::netkeep_protocol_handler =
	    crow::netkeep_protocol_handler_crowker;

	const struct option long_options[] = {{"udp", required_argument, NULL, 'u'},
		{"debug", no_argument, NULL, 'd'},
		{"binfo", no_argument, NULL, 'b'},
		{"vdebug", no_argument, NULL, 'v'},
		{"quite", no_argument, NULL, 'q'},
		{"handshake", required_argument, NULL, 'h'},
		{"netname", required_argument, NULL, 'n'},
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "usvdibhn", long_options,
	                          &long_index)) != -1)
	{
		switch (opt)
		{
			case 'u':
				udpport = atoi(optarg);
				break;
			case 'd':
				crow::enable_diagnostic();
				break;
			case 'v':
				crow::enable_live_diagnostic();
				break;
			case 'b':
				brocker_info = true;
				break;
			case 'h':
				handshake = optarg;
				break;
			case 'n':
				netname = optarg;
				break;
			case 0:
				break;
		}
	}

	std::thread netkeep_thread {netserve};
	netkeep_thread.detach();

	crow::set_netname(netname.c_str());

	if (!handshake.empty())
	{
		std::thread alive_thread {alivespam};
		alive_thread.detach();
	}

	if (udpport == -1)
	{
		printf("You should specify udp port.\n");
		exit(-1);
	}

	if (crow::create_udpgate(CROW_UDPGATE, udpport) == NULL)
	{
		perror("udpgate open");
		exit(-1);
	}

	crow::spin();
}
