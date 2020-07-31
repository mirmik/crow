#ifndef CROW_BROCKER_SERVICE_H
#define CROW_BROCKER_SERVICE_H

#include <crow/types.h>
#include <crow/address.h>
#include <crow/proto/node.h>
#include <crow/proto/request.h>

#include <unordered_map>
#include <unordered_set>

#define	CROW_SERVICE_ONESHOOT 0
#define	CROW_SERVICE_NODE_RETRANS 1
#define	CROW_SERVICE_CHANNEL_RETRANS 2
#define	CROW_SERVICE_CLOSE 3

#define	CROW_SERVICE_REGISTER_NODE 4
#define	CROW_SERVICE_REGISTER_CHANNEL 5

namespace crow
{
	struct nodeaddr_t 
	{
		std::vector<uint8_t> naddr;
		nid_t nid;
	};

	enum service_type_e : uint8_t
	{
		CROWKER_SERVICE_TYPE_NODE = 0,
		CROWKER_SERVICE_TYPE_CHANNEL = 1,
	};

	class crowker_service_node;

	struct crowker_service_callback_record
	{
		std::vector<uint8_t> host;
		nid_t                id;
		crowker_service_node * service;
	};

	void crowker_service_callback(void* arg, int sts, crow::packet * pack);
	
	class node_retransler;
	class channel_retransler;

	struct service_record
	{
		nodeaddr_t naddr;
		service_type_e srvtype;

		std::unordered_set<crow::oneshoot_async_requester *> requesters;
		std::unordered_set<crow::node_retransler *> node_retranslers;
		std::unordered_set<crow::channel_retransler *> channel_retranslers;
	};

	class crowker_service_control_node_cls : public crow::node
	{
		//using register_record_t = std::pair<nodeaddr_t, std::string name>;

		std::unordered_map<std::string, crow::nodeaddr_t> services;

		//std::vector<uint8_t> host;
		//nodeid_t             nid;
		//std::unordered_set<crow::hostaddr *>;

		crowker_service_control_node_cls()
		{}

		void incoming_packet(crow::packet * pack) override
		{
			/*auto cbrec = new crowker_service_callback_record(
				{pack->addrptr(), pack->addrsize()},
				node::subheader(pack)->sid);

			crow::async_request(callback, cbrec, { host.data(), host.size() }, nid,
				node::message(pack), 2, pack->ackquant());*/

			auto nodemsg = node::message(pack);
			igris::buffer saddr = pack->addr();
			auto sid = node::sid(pack);

			igris::binreader reader(nodemsg.data());

			uint8_t cmd;
			uint8_t namelen;
			char * name;

			reader.read_binary(cmd);
			reader.read_binary(namelen);
			reader.bind_buffer(name, namelen);

			switch (cmd)
			{
				case CROW_SERVICE_ONESHOOT:
				{
					uint16_t datalen;
					char * data;

					reader.read_binary(datalen);
					reader.bind_buffer(data, datalen);

					auto cbrec = new crowker_service_callback_record(
					{pack->addrptr(), pack->addrsize()},
					node::subheader(pack)->sid);

					crow::async_request(callback, cbrec, { host.data(), host.size() }, nid,
					                    node::message(pack), 2, pack->ackquant());
				}
				break;
				case CROW_SERVICE_NODE_RETRANS: BUG(); break;
				case CROW_SERVICE_CHANNEL_RETRANS: BUG(); break;
				case CROW_SERVICE_CLOSE: BUG(); break;

				case CROW_SERVICE_REGISTER_NODE:
				{
					services.insert(
					{name, namelen},
					new service_record(
					{{saddr.data(), saddr.size()}, sid},
					CROWKER_SERVICE_TYPE_NODE));
				}
				break;

				case CROW_SERVICE_REGISTER_CHANNEL:
					BUG();
					break;
			}

			crow::release(pack);
		}

		void undelivered_packet(crow::packet * pack) override
		{
			crow::release(pack);
		}
	};

	crowker_service_control_node_cls * crowker_service_control_node();
}

#endif