#ifndef CROW_NETPROTO_H
#define CROW_NETPROTO_H

/// netproto - алгоритм и субпротокол для объединения
/// нескольких брокеров в единую сеть.

#include <crow/tower.h>
#include <mutex>

#define CROW_BROCKER_ALIVE 7

#define CROW_TOWER_TYPE_COMMON 0
#define CROW_TOWER_TYPE_CROWKER 2

namespace crow
{
	struct alived_tower
	{
		std::string addr;
		std::string name;
		uint8_t type;

		chrono::timestamp lastalive;		
	};

	struct netproto_subheader
	{
		uint8_t code;
		uint8_t declared_name_len;
		uint8_t declared_type;
		uint8_t datlen;
	};

	extern std::list<alived_tower> alived_list;
	extern std::mutex netproto_mutex;

	static inline
	void netproto_handler(crow::packet * pack)
	{
		struct netproto_subheader * header = (struct netproto_subheader *)
		                                     (pack->rawdata().data());

		switch (header -> code)
		{
			case CROW_BROCKER_ALIVE_HANDSHAKE:
				netproto_alive_send(CROW_TOWER_TYPE_CROWKER, pack->addr());
				//fallthrow
			case CROW_BROCKER_ALIVE:
				netproto_mutex.lock();
				browker_list.insert(header->addrsect());
				netproto_mutex.unlock();
				break; 

			default:
				break;
		}

		crow::release(pack);
		return;
	}

	static inline
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

}

#endif