#ifndef CROW_PROTOCOL_H
#define CROW_PROTOCOL_H

#include <crow/packet.h>
#include <crow/tower.h>
#include <igris/datastruct/dlist.h>

namespace crow 
{
	class protocol 
	{
	public:
		struct dlist_head lnk = DLIST_HEAD_INIT(lnk);
		uint8_t id;

		virtual void incoming(crow::packet * pack) = 0;
		virtual void undelivered(crow::packet * pack) { crow::release(pack); }
		
		void enable();

		protocol(int id) : id(id) {}
	};
}

#endif