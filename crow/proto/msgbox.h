#ifndef CROW_MSGBOX_H
#define CROW_MSGBOX_H

#include <crow/proto/node.h>
#include <igris/datastruct/dlist.h>
#include <igris/sync/syslock.h>

#include <vector>

#define CROW_MSGBOX_STATE_NONE 0
#define CROW_MSGBOX_STATE_SEND 1
#define CROW_MSGBOX_STATE_RECEIVE 2

namespace crow
{
	class msgbox : public crow::node
	{
		struct dlist_head messages = DLIST_HEAD_INIT(messages);

	public:
		crow::node_packet_ptr query(uint16_t rid,
		                            const crow::hostaddr & addr,
		                            const igris::buffer data,
		                            uint8_t qos, uint16_t ackquant)
		{
			assert(dlist_empty(&messages));
			send(rid, addr, data, qos, ackquant);
			return receive();
		}

		crow::packet_ptr receive()
		{
			system_lock();
			while (dlist_empty(&messages))
			{
				int sts = waitevent();
				if (sts == -1)
					return nullptr;
			}

			crow::packet* pack = dlist_first_entry(&messages, crow::packet, ulnk);
			dlist_del_init(&pack->ulnk);
			system_unlock();

			return pack;
		}

		void incoming_packet(crow::packet *pack) override
		{
			system_lock();
			dlist_add_tail(&pack->ulnk, &messages);
			notify_one(0);
			system_unlock();
		}

		void undelivered_packet(crow::packet *pack) override
		{
			system_lock();
			notify_one(-1);
			crow::release(pack);
			system_unlock();
		}

		~msgbox()
		{
			system_lock();
			assert(dlist_empty(&messages));
			system_unlock();
		}
	};
}

#endif