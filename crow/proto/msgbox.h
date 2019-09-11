#ifndef CROW_MSGBOX_H
#define CROW_MSGBOX_H

#include <crow/proto/node.h>
#include <igris/datastruct/dlist.h>

#define CROW_MSGBOX_STATE_NONE 0
#define CROW_MSGBOX_STATE_SEND 1
#define CROW_MSGBOX_STATE_RECEIVE 2

namespace crow 
{
	class msgbox : public crow::node
	{
		uint8_t state = CROW_MSGBOX_STATE_NONE;
		struct dlist_head wq = DLIST_HEAD_INIT(wq);
		struct dlist_head messages = DLIST_HEAD_INIT(messages);

	public:
		void send(void* addr, uint8_t alen, uint16_t rid,
			void* data, size_t dlen, 
			uint8_t qos, uint16_t ackquant) 
		{
			state = CROW_MSGBOX_STATE_SEND;
			crow::node_send(sid, rid, addr, alen, data, dlen, qos, ackquant);
		}

		crow::packet* query() 
		{
			
		}		

		// Ожидать входящего сообщения.
		// Возвращает управления, если в очереди есть хотя бы один пакет
		// TODO: или, если вернулся таймаут.
		/*int wait_reply(uint16_t timeout) 
		{
			system_lock();

			while (dlist_empty(&messages)) 
			{
				wait_current_schedee(&wq);
			}

			system_unlock();
		}*/

		crow::packet* receive() 
		{
			system_lock();

			while (dlist_empty(&messages)) 
			{
				wait_current_schedee(&wq);
			}

			crow::packet* pack = dlist_first(&messages);
			dlist_del_init(&pack->lnk);

			system_unlock();

			return pack;
		}

		void reply(crow::packet * pack, 
			const void * data, uint16_t dlen, 
			uint8_t qos, uint16_t ackquant) 
		{

		}

		void incoming_packet(crow::packet *pack) override 
		{
			system_lock();
			dlist_add(&pack->lnk, &messages);
			unwait_one(&wq);
			system_unlock();
		}
	};
}

#endif