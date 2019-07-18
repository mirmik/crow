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
#include <igris/util/dstring.h>

#include <nos/inet/tcp_server.h>
#include <nos/inet/tcp_socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

std::string handshake;
std::string netname;

int udpport = -1;
int tcpport = -1;
bool quite = false;

void incoming_pubsub_packet(struct crow::packet *pack)
{
	struct crow_subheader_pubsub *shps = crow::get_subheader_pubsub(pack);

	switch (shps->type)
	{
		case PUBLISH:
			{
				auto theme = std::make_shared<std::string>
					(crow::pubsub::get_theme(pack));
				auto data = std::make_shared<std::string>
					(crow::pubsub::get_data(pack));

				brocker::publish(theme, data);
			}
			break;

		case SUBSCRIBE:
			{
				auto shps_c = get_subheader_pubsub_control(pack);
				std::string theme(pack->dataptr() + sizeof(crow_subheader_pubsub) +
				                  sizeof(crow_subheader_pubsub_control),
				                  shps->thmsz);

				brocker::crow_subscribe(pack->addrptr(), pack->addrsize(), theme,
				                        shps_c->qos, shps_c->ackquant);
			}
			break;

		default:
			{
				printf("unresolved pubsub frame type %d", (uint8_t)shps->type);
			}
			break;
	}

	//crow::release(pack);
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
			//auto &thm = brocker::themes[theme];

			brocker::erase_crow_subscriber(
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

void tcp_client_listener(nos::inet::tcp_socket client)
{
	char buf[1024];
	nos::inet::netaddr addr = client.getaddr();

	if (brocker_info)
		nos::println("new tcp client from", addr);

	while (1)
	{
		int ret;
		char cmd;
		uint8_t datasize;
		uint16_t thmsize;

		std::shared_ptr<std::string> theme;
		std::shared_ptr<std::string> data;

		ret = client.recv(buf, 3, MSG_WAITALL);

		if (ret <= 0)
			break;

		cmd = buf[0];
		if (cmd != 's' && cmd != 'p') 
			goto clean;

		buf[3] = 0;
		thmsize = atoi32(buf + 1, 10, nullptr);

		if (thmsize == 0) 
			goto clean;

		ret = client.recv(buf, thmsize, MSG_WAITALL);

		if (ret <= 0)
			break;

		theme = std::make_shared<std::string>(buf, thmsize);

		if ( cmd == 's' )
		{
			brocker::tcp_subscribe(*theme, &client);
			continue;
		}
		else if (cmd == 'p')
		{
			ret = client.recv(buf, 6, MSG_WAITALL);
			if (ret <= 0)
				break;

			buf[6] = 0;
			datasize = atoi32(buf, 10, nullptr);
			if (datasize == 0) goto clean;

			ret = client.recv(buf, datasize, MSG_WAITALL);
			if (ret <= 0)
				break;

			data = std::make_shared<std::string>(buf, datasize);

			brocker::publish(theme, data);
			continue;
		}

	clean:
		{
			if (brocker_info)
				nos::println("unresolved tcp command from", addr, cmd);

			continue;
		}
	}

	if (brocker_info)
		nos::println("tcp connection was clossed", addr);
}

void tcp_listener(int port)
{
	nos::inet::tcp_server srv;
	srv.init();
	srv.reusing(true);
	srv.bind("0.0.0.0", port);
	srv.listen(10);

	while (1)
	{
		auto client = srv.accept();

		std::thread thr(tcp_client_listener, std::move(client));
		thr.detach();
	}
}

int main(int argc, char *argv[])
{
	crow::pubsub_protocol.incoming_handler = incoming_pubsub_packet;
	crow::undelivered_handler = undelivered_handler;

	//crow::pubsub_protocol.enable();

	crow::netkeep_protocol_handler =
	    crow::netkeep_protocol_handler_crowker;

	const struct option long_options[] =
	{
		{"udp", required_argument, NULL, 'u'},
		{"tcp", required_argument, NULL, 't'},
		{"debug", no_argument, NULL, 'd'},
		{"binfo", no_argument, NULL, 'b'},
		{"logpub", no_argument, NULL, 'p'},
		{"vdebug", no_argument, NULL, 'v'},
		{"quite", no_argument, NULL, 'q'},
		{"handshake", required_argument, NULL, 'h'},
		{"netname", required_argument, NULL, 'n'},
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;

	while ((opt = getopt_long(argc, argv, "usvdibhtn", long_options,
	                          &long_index)) != -1)
	{
		switch (opt)
		{
			case 'u':
				udpport = atoi(optarg);
				break;

			case 't':
				tcpport = atoi(optarg);
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

			case 'p':
				log_publish = true;
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


	if (tcpport != -1)
	{
		std::thread thr(tcp_listener, tcpport);
		thr.detach();
	}

	//crow::spin();
	while(1) 
	{
		crow::onestep();
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}
