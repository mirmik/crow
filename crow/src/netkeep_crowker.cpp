#include <crow/netkeep.h>
#include <crow/alive.h>

#include <igris/container/dualmap.h>

#include <mutex>
#include <chrono>

#include <string>
#include <list>

struct alived_tower
{
	uint8_t type;
	std::chrono::time_point<std::chrono::system_clock> lastalive;
};

using alivemap_t =
    igris::dualmap <std::string, std::string, struct alived_tower>;
alivemap_t alivemap;

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
		{
			std::string addr = (std::string) pack->addr();
			std::string name { (char*)(header + 1), header->nlen };

			std::lock_guard<std::mutex> lock(netproto_mutex);

			if (alivemap.contains(std::make_pair(addr, name)))
			{
				dprln("NETPROTO:CONTAINS");
				alivemap.at(std::make_pair(addr, name)).lastalive =
				    std::chrono::system_clock::now();
			}
			else
			{
				dprln("NETPROTO:CREATENEW");
				alived_tower record
				{
					header->type, std::chrono::system_clock::now() };
				alivemap.insert(addr, name, record);
			}
		};
		break;

		default:
			break;
	}

	crow::release(pack);
	return;
}

void crow::netkeep_serve()
{
	std::chrono::time_point<std::chrono::system_clock> now;
	alivemap_t::iter0 it, eit, next;
	std::lock_guard<std::mutex> lock(netproto_mutex);

	while (true)
	{
		it = alivemap.begin0();
		next = std::next(it);
		eit = alivemap.end0();

		now = std::chrono::system_clock::now();

		/*for (; it != eit; ++it)
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
			alivemap.erase(it);
			//browker_list::remove(it);
		}*/
	}
}

/*static inline
void debug_print_brocker_list()
{
	for (const auto& s : browker_list)
	{
		dprln(s.first);
	}
}
*/