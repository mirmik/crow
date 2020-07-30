#ifndef CROW_BROCKER_SERVICE_H
#define CROW_BROCKER_SERVICE_H

#include <crow/node.h>

namespace crow 
{
	class crowker_service_node;

	struct crowker_service_callback_record 
	{
		std::vector<uint8_t> host;
		nid_t                id;
		crowker_service_node * service; 	
	};

	static
	void callback(void* arg, int sts, crow::packet * pack) 
	{
		crowker_service_callback_record * cbrec = (crowker_service_callback_record *) arg;
		cbrec->service->send(
			{ cbrec->host.data(), cbrec->host.size() }, 
			cbrec->id,
			node::message(pack),
			pack->qos(),
			pack->ackquant()
		);

		delete cbrec;
	}

	class crowker_service_node : public crow::node 
	{
		std::vector<uint8_t> host;
		nodeid_t             nid;
		std::unordered_set<crow::oneshoot_async_requester *>;

		crowker_service_node(crow::hostaddr host, nodeid_t nid) 
			: host(host.data(), host.size()), nid(nid)
		{}

		void incoming_packet(crow::packet * pack) override 
		{
			auto cbrec = new crowker_service_callback_record(
				{pack->addrptr(), pack->addrsize()}, 
				node::subheader(pack)->sid);

			crow::async_request(callback, cbrec, { host.data(), host.size() }, nid, 
				node::message(pack), 2, pack->ackquant());
		}

		void undelivered_packet(crow::packet * pack) override 
		{
			crow::release(pack);
		}
	};

	extern std::unordered_map<crowker_service_node> services;
}

#endif