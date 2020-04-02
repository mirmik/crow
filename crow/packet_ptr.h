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

		bool operator == (std::nullptr_t) 
		{
			return pack == nullptr;
		}
	};
}

#endif