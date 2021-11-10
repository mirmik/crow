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

namespace crow
{
    /**
        @brief Структура заголовок пакета.
        @details Заголовок пакета располагается в первых байтах пакета.
        за заголовком следует поле адреса переменной длины, а за ним данные.
    */
    struct header_v1
    {
        union _u
        {
            uint8_t pflag = 0; ///< Флаги пакета
            struct _f
            {
                uint8_t ack : 1; ///< Идентифицирует ack пакеты. Доп.инф.
                ///< передается в типе.
                uint8_t RESERVED1 : 1;
                uint8_t RESERVED2 : 1;
                uint8_t type : 5; ///< Доп. инф. зависит от ситуации.
            } f;
        } u;
        uint16_t flen; ///< Полная длина пакета
        uint8_t alen;  ///< Длина поля адреса.
        uint8_t stg; ///< Поле стадии. Используется для того, чтобы цепочка врат
        ///< знала, какую часть адреса обрабатывать.
        uint16_t ackquant; ///< Таймаут для пересылки пакета.
        uint16_t
            seqid; ///< Порядковый номер пакета. Присваивается отправителем.
        uint8_t qos; ///< Поле качества обслуживания.
    } __attribute__((packed));

    class packet
    {
    public:
        struct dlist_head lnk =
            DLIST_HEAD_INIT(lnk); ///< Для подключения в списки башни crow.
        struct dlist_head ulnk =
            DLIST_HEAD_INIT(ulnk); ///< Для подключения в список пользователя и
        ///< зависимых протоколов.
        crow::gateway *ingate; ///< gate, которым пакет прибыл в систему.
        uint16_t last_request_time; ///< время последней отправки
        uint16_t _ackcount; ///< счетчик количества попыток отправки
        int8_t refs;
        union _u
        {
            uint8_t flags = 0; ///< Местные флаги
            struct _f
            {
                uint8_t released_by_world : 1;
                uint8_t released_by_tower : 1;
                uint8_t confirmed : 1;
                uint8_t undelivered : 1;
                uint8_t sended_to_gate : 1;
            } f;
        } u;

    public:
        virtual void revert_gate(uint8_t gateindex) = 0;
        virtual void revert(igris::buffer *vec, size_t veclen) = 0;

        virtual uint8_t *addrptr() = 0;
        virtual uint8_t addrsize() = 0;

        virtual char *dataptr() = 0;
        virtual uint16_t datasize() = 0;

        virtual char *endptr() = 0;
        virtual uint8_t *stageptr() = 0;

        virtual header_v1 &header() = 0;

        igris::buffer addr() { return {addrptr(), addrsize()}; }
        igris::buffer data() { return {dataptr(), datasize()}; }

        template <class T> T &subheader()
        {
            return *reinterpret_cast<T *>(dataptr());
        }
    };

    class compacted_packet : public packet
    {
    public:
        header_v1 _header;

    public:
        header_v1 &header() override { return _header; }

        void revert_gate(uint8_t gateindex) override;
        void revert(igris::buffer *vec, size_t veclen) override;

        uint8_t *addrptr() override;
        uint8_t addrsize() override;

        char *dataptr() override;
        uint16_t datasize() override;

        char *endptr() override;

        uint8_t *stageptr() override
        {
            return (uint8_t *)(&header() + 1) + header().stg;
        }
    };
}

extern int crow_allocated_count;

__BEGIN_DECLS

void crow_packet_initialization(crow::compacted_packet *pack,
                                crow::gateway *ingate);

crow::compacted_packet *crow_create_packet(crow::gateway *ingate,
                                           uint8_t addrsize, size_t datasize);

/**
 * Выделить память для пакета.
 *
 * Выделяет adlen + sizeof(crow::packet) байт
 * @param adlen Суммарная длина адреса и данных в выделяемом пакете.
 */
crow::compacted_packet *crow_allocate_packet(size_t adlen);

///Вернуть память выделенную для пакета pack
void crow_deallocate_packet(crow::packet *pack);

__END_DECLS

namespace crow
{
    // Только для аллокации через pool.
    void engage_packet_pool(void *zone, size_t zonesize, size_t elsize);
    igris::pool *get_package_pool();

    bool has_allocated();

    void diagnostic(const char *label, crow::packet *pack);
}

#endif
