#ifndef CROW_NODE_SPAMMER_H
#define CROW_NODE_SPAMMER_H

#include <crow/proto/node.h>
#include <chrono>

namespace crow 
{
	class spammer 
	{
		class record 
		{
			std::chrono::system_clock::tstamp last_subscribe;
		};

		std::map<nodeaddr, record> targets;

		unsigned int timeout = 10000;
		uint8_t qos = 0;
		uint16_t ackquant = 50;
		//std::vector<crow::nodeaddr> targets;

		void send(igris::buffer data) 
		{
			auto time = std::chrono::system_clock::now();

			std::vector<std::map<nodeaddr, record>::iterator> to_delete;

			auto eit = targets.end();
			auto it = targets.begin()
			for (; it != eit; it++) 
			{
				if (time - t.second.last_subscribe > timeout) 
				{
					to_delete.push_back(it);
					continue;
				}

				node::send(
					t.nid, 
					t.addr.to_hostaddr(), 
					data,
					qos,
					ackquant);
			}

			for (auto it : to_delete) 
			{
				targets.remove(it);
				nos::println("TO_DELETE", targets.size());
			}
		}

		void incoming_handler(crow::packet * pack) override
		{	
			auto time = std::chrono::system_clock::now();


			crow::release();
		}
	};

	class spam_subscriber : public crow::node
	{
		igris::delegate<void, igris::buffer> dlg;

		void subscribe(nid_t nid, crow::hostaddr_t host, uint8_t qos=2, uint16_t ackquant=200) 
		{
			node::send(nid, host, "", qos, ackquant);
		}		

		void incoming_handler(crow::packet * pack) 
		{
			
		}
	};
}

#endif