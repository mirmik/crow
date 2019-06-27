/**
@file
@brief Всё, что касается работы с пакетом.
*/

#ifndef CROW_PACKAGE_H
#define CROW_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/uio.h>

#include <crow/defs.h>
#include <igris/datastruct/dlist.h>

#include <igris/buffer.h>

/// Качество обслуживания.
#define CROW_WITHOUT_ACK 0
#define CROW_TARGET_ACK 1
#define CROW_BINARY_ACK 2

namespace crow
{
	struct gateway;

	/**
		@brief Структура заголовок пакета.
		@details Заголовок пакета располагается в первых байтах пакета.
		за заголовком следует поле адреса переменной длины, а за ним данные.
	*/
	struct header
	{
		union {
			uint8_t pflag; ///< Флаги пакета
			struct
			{
				uint8_t ack : 1; ///< Идентифицирует ack пакеты. Доп.инф.
								 ///< передается в типе.
				uint8_t RESERVED : 1;
				uint8_t noexec : 1; ///< Флаг предотвращает исполнение пакета.
									///< Используется для запросов существования
				uint8_t type : 5; ///< Доп. инф. зависит от ситуации.
			} f;
		};
		uint16_t flen; ///< Полная длина пакета
		uint8_t alen;  ///< Длина поля адреса.
		uint8_t stg; ///< Поле стадии. Используется для того, чтобы цепочка врат
					 ///< знала, какую часть адреса обрабатывать.
		uint16_t ackquant; ///< Таймаут для пересылки пакета.
		uint16_t seqid; ///< Порядковый номер пакета. Присваивается отправителем.
		uint8_t qos; ///< Поле качества обслуживания.
	} __attribute__((packed));

	struct packet
	{
		struct dlist_head lnk; ///< Для подключения в списки башни crow.
		struct dlist_head ulnk; ///< Для подключения в список пользователя и
								///< зависимых протоколов.
		struct crow::gateway *ingate; ///< gate, которым пакет прибыл в систему.
		uint16_t last_request_time; ///< @todo
		uint8_t ackcount;			///< @todo
		uint8_t refs;
		union {
			uint8_t flags; ///< Местные флаги
			struct
			{
				uint8_t released_by_world : 1;
				uint8_t released_by_tower : 1;
			} f;
		};
		struct crow::header header;

		uint8_t *stageptr() { return (uint8_t *)(&header + 1) + header.stg; }
		// uint8_t& stage() { return *stageptr(); }

		char *endptr() { return (char *)(&header) + header.flen; }
		uint16_t blocksize() { return header.flen; }

		[[deprecated]] igris::buffer datasect() {
			return igris::buffer(dataptr(), datasize());
		}

			[[deprecated]] igris::buffer addrsect()
		{
			return igris::buffer(addrptr(), addrsize());
		}

		igris::buffer rawdata() { return igris::buffer(dataptr(), datasize()); }
		igris::buffer addr() { return igris::buffer(addrptr(), addrsize()); }

		void revert_gate(uint8_t gateindex);
		void revert(struct iovec *vec, size_t veclen);

		uint8_t *addrptr() { return (uint8_t *)(&header + 1); }
		uint8_t addrsize() { return header.alen; }

		char *dataptr() { return (char *)(&header + 1) + header.alen; }
		uint16_t datasize()
		{
			return (uint16_t)(header.flen - header.alen - sizeof(crow::header));
		}

		size_t fullsize() 
		{
			return header.flen;
		};
	}; //На самом деле, он не должен быть packed.
	//__attribute__((packed));

	/**
	 * Выделить память для пакета.
	 *
	 * Выделяет adlen + sizeof(crow::packet) байт
	 * @param adlen Суммарная длина адреса и данных в выделяемом пакете.
	 */
	crow::packet *allocate_packet(size_t adlen);

	///Вернуть память выделенную для пакета pack
	void deallocate_packet(crow::packet *pack);

	packet *create_packet(struct crow::gateway *ingate, uint8_t addrsize,
						  size_t datasize);

	void packet_initialization(struct crow::packet *pack,
							   struct crow::gateway *ingate);

	void diagnostic(const char *notation, crow::packet *pack);

	void engage_packet_pool(void *zone, size_t zonesize, size_t elsize);

	extern int allocated_count;
	bool has_allocated();

} // namespace crow

/**
 * Выделить память для пакета.
 *
 * Выделяет adlen + sizeof(crow::packet) байт
 * @param adlen Суммарная длина адреса и данных в выделяемом пакете.
 */
/*crow::packet* crow_allocate_packet(size_t adlen);

///Вернуть память выделенную для пакета pack
void crow_deallocate_packet(crow::packet* pack);

///
crow::packet* crow_create_packet(struct crow::gateway* ingate, size_t addrsize,
size_t datasize);

///
void crow_utilize(crow::packet* pack);

void crow_print(crow::packet* pack);
void crow_println(crow::packet* pack);

__END_DECLS

#ifdef __cplusplus

namespace crow
{
	static inline void utilize(crow::packet* pack) { crow_utilize(pack); }
}*/

#endif