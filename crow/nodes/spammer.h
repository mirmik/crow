#ifndef CROW_NODE_SPAMMER_H
#define CROW_NODE_SPAMMER_H

#include <crow/extra/nodeaddr.h>
#include <crow/proto/node.h>
#include <chrono>
#include <map>

#include <igris/event/delegate.h>

#include <nos/print.h>

using namespace std::literals::chrono_literals;

namespace crow
{
	class spammer : public crow::node
	{
		struct record
		{
			std::chrono::time_point<std::chrono::system_clock> last_subscribe;
		};

		std::map<nodeaddr, record> targets;

		std::chrono::milliseconds timeout = 5000ms;
		uint8_t qos = 0;
		uint16_t ackquant = 50;
	
	public:
		void send(igris::buffer data)
		{
			auto time = std::chrono::system_clock::now();

			std::vector<std::map<nodeaddr, record>::iterator> to_delete;

			auto eit = targets.end();
			auto it = targets.begin();
			for (; it != eit; it++)
			{
				if (time - it->second.last_subscribe > timeout)
				{
					to_delete.push_back(it);
					continue;
				}

				node::send(
				    it->first.nid,
				    it->first.hostaddr(),
				    data,
				    qos,
				    ackquant);
			}

			for (auto it : to_delete)
			{
				targets.erase(it);
			}
		}

		void send_v(igris::buffer * data, size_t sz) 
		{

			auto time = std::chrono::system_clock::now();

			std::vector<std::map<nodeaddr, record>::iterator> to_delete;

			auto eit = targets.end();
			auto it = targets.begin();
			for (; it != eit; it++)
			{
				if (time - it->second.last_subscribe > timeout)
				{
					to_delete.push_back(it);
					continue;
				}

				node::send_v(
				    it->first.nid,
				    it->first.hostaddr(),
				    data,
				    sz,
				    qos,
				    ackquant);
			}

			for (auto it : to_delete)
			{
				targets.erase(it);
			}
		}

		void incoming_packet(crow::packet * pack) override
		{
			auto time = std::chrono::system_clock::now();

			std::vector<uint8_t> addr(pack->addrptr(), pack->addrptr() + pack->addrsize());
			targets[nodeaddr{addr, node::sid(pack)}] = record{time};

			crow::release(pack);
		}

		int count_of_subscribers() 
		{
			return targets.size();
		}
	};

	class spam_subscriber : public crow::node
	{
		igris::delegate<void, igris::buffer> dlg;

	public:
		spam_subscriber(igris::delegate<void, igris::buffer> dlg) : dlg(dlg) {}

		void subscribe(nid_t nid, crow::hostaddr host, uint8_t qos = 2, uint16_t ackquant = 200)
		{
			node::send(nid, host, "", qos, ackquant);
		}

		void incoming_packet(crow::packet * pack) override
		{
			dlg(node::message(pack));
			crow::release(pack);
		}
	};
}

#endif