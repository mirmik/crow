/**
	@file
*/

#ifndef G1_GATEWAY_H
#define G1_GATEWAY_H

#include <crow/packet.h>
#include <gxx/container/dlist.h>

namespace crow {
	/**
		@brief Абстрактный класс врат. Врата отвечают за пересылку пакетов между мирами.
		@details Может содержать поле списка и некоторое время хранить отправляемые пакеты.
	*/
	struct gateway {
		dlist_head lnk; ///< встроенное поле списка.
		uint16_t id; ///< номер врат.
		
		gateway() {
			dlist_init(&lnk);
		}
		
		/** 
			@brief Отправить пакет в целевой мир, согласно адресу стадии.
			@details Убить зверя.
			@param pack Пересылаемый пакет
			@return Статус ошибки.
		*/
		virtual void send(packet* pack) = 0;

		virtual void nonblock_onestep() {};
	};

}

#endif