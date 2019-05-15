#include <crow/netkeep.h>
#include <crow/alive.h>

#include <mutex>
#include <chrono>

#include <string>
#include <list>

struct alived_tower
{
	std::string addr;
	std::string name;
	uint8_t type;

	std::chrono::time_point<std::chrono::system_clock> lastalive;
};

std::list<alived_tower> alived_list;
std::mutex netproto_mutex;

void crow::netkeep_protocol_handler_crowker(crow::packet * pack)
{
	struct alive_header * header = (struct alive_header *)
	                               (pack->rawdata().data());

	switch (header -> code)
	{
		case CROW_ALIVE_HANDSHAKE:
			send_alive_message(pack->addrptr(), pack->addrsize(),
			                   CROW_ALIVE, CROW_TOWER_TYPE_CROWKER,
			                   pack->header.qos, pack->header.ackquant);
		//fallthrow
		case CROW_ALIVE:
			//netproto_mutex.lock();
			//browker_list.insert(header->addrsect());
			//netproto_mutex.unlock();
			break;

		default:
			break;
	}

	crow::release(pack);
	return;
}

/*	static inline
	void crow::netproto_serve()
	{
		auto now;
		std::map<std::string, chrono::timestamp>::iterator it, eit;

		std::lock_guard<std::mutex> lock(netproto_mutex);

		while (true)
		{
			it = browker_list.begin();
			eit = browker_list.end();
			now = chrono::sysclock::now();

			for (; it != eit; ++it)
			{
				if (s.second - now > 8s)
				{
					break;
				}
			}

			if (it == eit)
				break;

			else
			{
				browker_list::remove(it);
			}
		}
	}

	static inline
	void debug_print_brocker_list()
	{
		for (const auto& s : browker_list)
		{
			dprln(s.first);
		}
	}
*/