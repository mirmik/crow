/**
	@file
*/

#ifndef G1_GATEWAY_H
#define G1_GATEWAY_H

#include <crow/packet.h>
#include <igris/datastruct/dlist.h>

// struct crow::gateway_operations;

/**
	@brief Абстрактный класс врат. Врата отвечают за пересылку пакетов между
   мирами.
	@details Может содержать поле списка и некоторое время хранить отправляемые
   пакеты.
*/
namespace crow
{
	struct gateway
	{
		struct dlist_head lnk; ///< встроенное поле списка.
		uint8_t id;			   ///< номер врат.

		virtual void send(crow::packet *) = 0;
		virtual void nblock_onestep() = 0;
	};
} // namespace crow

#endif