#ifndef CROW_MSGBOX_H
#define CROW_MSGBOX_H

#include <crow/proto/node.h>
#include <igris/datastruct/dlist.h>
#include <igris/sync/syslock.h>

#define CROW_MSGBOX_STATE_NONE 0
#define CROW_MSGBOX_STATE_SEND 1
#define CROW_MSGBOX_STATE_RECEIVE 2

namespace crow 
{
	class msgbox : public crow::node
	{
		struct dlist_head messages = DLIST_HEAD_INIT(messages);

	public:
		void send(void* addr, uint8_t alen, uint16_t rid,
			void* data, size_t dlen, 
			uint8_t qos, uint16_t ackquant) 
		{
		//	state = CROW_MSGBOX_STATE_SEND;
			crow::node_send(id, rid, addr, alen, data, dlen, qos, ackquant);
		}

		crow::packet* query(void* addr, uint8_t alen, uint16_t rid,
			void* data, size_t dlen, 
			uint8_t qos, uint16_t ackquant) 
		{
			assert(dlist_empty(&messages));
			send(addr, alen, rid, data, dlen, qos, ackquant);
			return receive();
		}		

		crow::packet* receive() 
		{
			system_lock();

			while (dlist_empty(&messages)) 
			{
				int sts = waitevent();
				if (sts == -1) return nullptr;
			}

			crow::packet* pack = dlist_first_entry(&messages, crow::packet, lnk);
			dlist_del_init(&pack->lnk);

			system_unlock();

			return pack;
		}

		void incoming_packet(crow::packet *pack) override 
		{
			system_lock();
			dlist_add_tail(&pack->lnk, &messages);
			notify_one(0);
			system_unlock();
		}

		void undelivered_packet(crow::packet *pack) override 
		{
			system_lock();
			notify_one(-1);
			system_unlock();
		}
	};
}

#endif