#ifndef CROW_PACKET_PTR_H
#define CROW_PACKET_PTR_H

#include <crow/packet.h>

namespace crow
{
	class packet_ptr
	{
	protected:
		crow::packet *pack;

	public:
		packet_ptr(crow::packet *pack_) : pack(pack_)
		{
			if (pack == nullptr) return;

			pack->refs++;
		}

		packet_ptr(const crow::packet_ptr &oth) : pack(oth.pack)
		{
			pack->refs++;
		}

		packet_ptr(crow::packet_ptr &&oth) : pack(oth.pack)
		{
			oth.pack = nullptr;
		}

		crow::packet* get()
		{
			return pack;
		}

		crow::packet* get() const
		{
			return pack;
		}

		crow::packet* operator ->() 
		{
			return pack;
		}

		crow::packet& operator *() 
		{
			return *pack;
		}

		~packet_ptr();

		operator bool() { return pack != nullptr; }

//		VALUE_GETTER(addr, pack->addr());
//		VALUE_GETTER(rawdata, pack->rawdata());
//		VALUE_GETTER(qos, pack->header.qos);
//		VALUE_GETTER(ackquant, pack->header.ackquant);
	};
}

#endif