#ifndef CROW_HOLDERS_H
#define CROW_HOLDERS_H

#include <crow/packet.h>
#include <crow/tower.h>

namespace crow
{
	class packref
	{
		crow::packet * pack;

		packref(crow::packet* pack_) : pack(pack_)
		{
			pack->refs++;
		}

		packref(const crow::packref & oth) : pack(oth.pack)
		{
			pack->refs++;
		}

		packref(crow::packref && oth) : pack(oth.pack)
		{
			oth.pack = nullptr;
		}

		~packref()
		{
			if (pack)
			{
				pack->refs--;

				if (pack->refs == 0)
					crow::release(pack);
			}
		}

		gxx::buffer rawdata() 
		{
			return pack->rawdata();
		}

		gxx::buffer addr() 
		{
			return pack->addr();
		}
	};
}

#endif