/**
@file
@brief Всё, что касается работы с пакетом.
*/

#ifndef CROW_PACKAGE_H
#define CROW_PACKAGE_H

#include <stdint.h>
#include <stdlib.h>

#include <crow/defs.h>
#include <crow/hostaddr_view.h>

#include <igris/buffer.h>
#include <igris/container/pool.h>
#include <igris/datastruct/dlist.h>

/// Качество обслуживания.
#define CROW_WITHOUT_ACK 0
#define CROW_TARGET_ACK 1
#define CROW_BINARY_ACK 2

namespace crow
{
    class gateway;
}

/**
    @brief Структура заголовок пакета.
    @details Заголовок пакета располагается в первых байтах пакета.
    за заголовком следует поле адреса переменной длины, а за ним данные.
*/
struct crow_header
{
    union
    {
        uint8_t pflag; ///< Флаги пакета
        struct
        {
            uint8_t ack : 1; ///< Идентифицирует ack пакеты. Доп.инф.
            ///< передается в типе.
            uint8_t RESERVED1 : 1;
            uint8_t RESERVED2 : 1;
            uint8_t type : 5; ///< Доп. инф. зависит от ситуации.
        } f;
    };
    uint16_t flen; ///< Полная длина пакета
    uint8_t alen;  ///< Длина поля адреса.
    uint8_t stg; ///< Поле стадии. Используется для того, чтобы цепочка врат
    ///< знала, какую часть адреса обрабатывать.
    uint16_t ackquant; ///< Таймаут для пересылки пакета.
    uint16_t seqid; ///< Порядковый номер пакета. Присваивается отправителем.
    uint8_t qos;    ///< Поле качества обслуживания.
} __attribute__((packed));

struct crow_packet
{
    struct dlist_head lnk =
        DLIST_HEAD_INIT(lnk); ///< Для подключения в списки башни crow.
    struct dlist_head ulnk =
        DLIST_HEAD_INIT(ulnk); ///< Для подключения в список пользователя и
    ///< зависимых протоколов.
    crow::gateway *ingate; ///< gate, которым пакет прибыл в систему.
    uint16_t last_request_time; ///< время последней отправки
    uint16_t _ackcount; ///< счетчик количества попыток отправки
    int8_t refs;
    union
    {
        uint8_t flags; ///< Местные флаги
        struct
        {
            uint8_t released_by_world : 1;
            uint8_t released_by_tower : 1;
            uint8_t confirmed : 1;
            uint8_t undelivered : 1;
            uint8_t sended_to_gate : 1;
        } f;
    };
    struct crow_header header;
};

extern int crow_allocated_count;

__BEGIN_DECLS

static inline uint8_t *crow_packet_stageptr(struct crow_packet *pack)
{
    return (uint8_t *)(&pack->header + 1) + pack->header.stg;
}

void crow_packet_revert_gate(struct crow_packet *pack, uint8_t gateindex);
void crow_packet_revert(struct crow_packet *pack, igris::buffer *vec,
                        size_t veclen);

uint8_t *crow_packet_addrptr(struct crow_packet *pack);
uint8_t crow_packet_addrsize(struct crow_packet *pack);

char *crow_packet_dataptr(struct crow_packet *pack);
uint16_t crow_packet_datasize(struct crow_packet *pack);

void crow_packet_initialization(struct crow_packet *pack,
                                crow::gateway *ingate);

struct crow_packet *crow_create_packet(crow::gateway *ingate, uint8_t addrsize,
                                       size_t datasize);

/**
 * Выделить память для пакета.
 *
 * Выделяет adlen + sizeof(crow_packet) байт
 * @param adlen Суммарная длина адреса и данных в выделяемом пакете.
 */
struct crow_packet *crow_allocate_packet(size_t adlen);

///Вернуть память выделенную для пакета pack
void crow_deallocate_packet(struct crow_packet *pack);

__END_DECLS

namespace crow
{
    // Только для аллокации через pool.
    void engage_packet_pool(void *zone, size_t zonesize, size_t elsize);
    igris::pool *get_package_pool();

    bool has_allocated();

    void diagnostic(const char *label, crow_packet *pack);
}

#endif
