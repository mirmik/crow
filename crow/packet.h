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
#include <crow/qosbyte.h>

#include <igris/buffer.h>
#include <igris/container/pool.h>
#include <igris/datastruct/dlist.h>
#include <igris/sync/syslock.h>

/// Качество обслуживания.
#define CROW_WITHOUT_ACK 0
#define CROW_TARGET_ACK 1
#define CROW_BINARY_ACK 2

extern int crow_allocated_count;

namespace crow
{
    class gateway;
    class packet;
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
                uint8_t stg : 3;
                uint8_t type : 3; ///< Доп. инф. зависит от ситуации.
            } f;
        } u = {};
        uint16_t flen = 0; ///< Полная длина пакета
        uint8_t alen = 0;  ///< Длина поля адреса.
        uint8_t stg =
            0; ///< Поле стадии. Используется для того, чтобы цепочка врат
        ///< знала, какую часть адреса обрабатывать.
        uint16_t seqid = 0; ///< Порядковый номер пакета.
        qosbyte _qos = {}; ///< Поле качества обслуживания.

        void set_qos(uint8_t quality)
        {
            _qos.set_qos(quality);
        }

        void set_ackquant(uint16_t quant)
        {
            _qos.set_ackquant(quant);
        }

        uint8_t qos() const
        {
            return _qos.qos();
        }

        uint16_t ackquant() const
        {
            return _qos.ackquant();
        }

        uint16_t addrsize()
        {
            return alen;
        }

        uint16_t datasize()
        {
            return flen - alen - sizeof(header_v1);
        }
    } __attribute__((packed));

    struct header_v0
    {
        union _u
        {
            uint8_t pflag = 0; ///< Флаги пакета
            struct _f
            {
                uint8_t ack : 1; ///< Идентифицирует ack пакеты. Доп.инф.
                ///< передается в типе.
                uint8_t RESERVED : 1;
                uint8_t noexec : 1; ///< Флаг предотвращает исполнение пакета.
                ///< Используется для запросов существования
                uint8_t type : 5; ///< Доп. инф. зависит от ситуации.
            } f;
        } u = {};
        uint16_t flen = 0; ///< Полная длина пакета
        uint8_t alen = 0;  ///< Длина поля адреса.
        uint8_t stg =
            0; ///< Поле стадии. Используется для того, чтобы цепочка врат
        ///< знала, какую часть адреса обрабатывать.
        uint16_t _ackquant = 0; ///< Таймаут для пересылки пакета.
        uint16_t seqid =
            0; ///< Порядковый номер пакета. Присваивается отправителем.
        uint8_t _qos = 0; ///< Поле качества обслуживания.

        void set_qos(uint8_t quality)
        {
            _qos = quality;
        }

        void set_ackquant(uint16_t quant)
        {
            _ackquant = quant;
        }

        uint8_t qos() const
        {
            return _qos;
        }

        uint16_t ackquant() const
        {
            return _ackquant;
        }

        uint16_t addrsize()
        {
            return alen;
        }

        uint16_t datasize()
        {
            return flen - alen - sizeof(header_v1);
        }
    } __attribute__((packed));

    class packet
    {
    public:
        struct dlist_head lnk =
            DLIST_HEAD_INIT(lnk); ///< Для подключения в списки башни crow.
        struct dlist_head ulnk =
            DLIST_HEAD_INIT(ulnk); ///< Для подключения в список пользователя и
        ///< зависимых протоколов.
        crow::gateway *ingate =
            nullptr; ///< gate, которым пакет прибыл в систему.
        uint16_t last_request_time = 0; ///< время последней отправки
        uint16_t _ackcount = 0; ///< счетчик количества попыток отправки
        int8_t refs = 0;
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
        } u = {};

        void (*destructor)(packet *) = nullptr;

    public:
        void set_destructor(void (*destructor)(packet *))
        {
            this->destructor = destructor;
        }

        auto get_destructor()
        {
            return destructor;
        }

        void revert_gate(uint8_t gateindex);
        void revert(igris::buffer *vec, size_t veclen);

        virtual uint8_t *addrptr() = 0;
        virtual uint8_t addrsize() = 0;
        virtual char *dataptr() = 0;
        virtual uint16_t datasize() = 0;
        virtual char *endptr() = 0;
        virtual uint8_t *stageptr() = 0;

        virtual uint16_t full_length() = 0;
        virtual uint8_t type() = 0;
        virtual uint8_t quality() = 0;
        virtual uint16_t ackquant() = 0;
        virtual uint8_t stage() = 0;
        virtual uint16_t seqid() = 0;
        virtual uint8_t ack() = 0;

        virtual void set_addrsize(uint8_t) = 0;
        virtual void set_datasize(uint16_t) = 0;
        virtual void set_type(uint8_t) = 0;
        virtual void set_quality(uint8_t) = 0;
        virtual void set_ackquant(uint16_t) = 0;
        virtual void set_stage(uint8_t) = 0;
        virtual void set_seqid(uint16_t) = 0;
        virtual void set_ack(uint8_t) = 0;

        void increment_stage(int i)
        {
            set_stage(stage() + i);
        }
        void increment_seqid(int i)
        {
            set_seqid(seqid() + i);
        }

        virtual void invalidate() = 0;
        virtual ~packet() = default;
        virtual void self_init() = 0;

        void parse_header(const header_v1 &h)
        {
            set_ack(h.u.f.ack);
            set_type(h.u.f.type);
            set_quality(h.qos());
            set_ackquant(h.ackquant());
            set_seqid(h.seqid);
            set_stage(h.stg);
            set_addrsize(h.alen);
            set_datasize(h.flen - h.alen - sizeof(header_v1));
        }

        header_v1 extract_header_v1()
        {
            header_v1 h;
            h.seqid = seqid();
            h.u.f.ack = ack();
            h.u.f.type = type();
            h.set_qos(quality());
            h.set_ackquant(ackquant());
            h.stg = stage();
            h.flen = datasize() + addrsize() + sizeof(header_v1);
            h.alen = addrsize();
            h.stg = stage();
            return h;
        }

        header_v0 extract_header_v0()
        {
            header_v0 h;
            h.seqid = seqid();
            h.u.f.ack = ack();
            h.u.f.type = type();
            h.set_qos(quality());
            h.set_ackquant(ackquant());
            h.stg = stage();
            h.flen = datasize() + addrsize() + sizeof(header_v0);
            h.alen = addrsize();
            h.stg = stage();
            return h;
        }

        crow::hostaddr_view addr()
        {
            return crow::hostaddr_view(addrptr(), addrsize());
        }

        igris::buffer data()
        {
            return igris::buffer(dataptr(), datasize());
        }

        template <class T> T &subheader()
        {
            return *reinterpret_cast<T *>(dataptr());
        }
    };

    class morph_packet : public packet
    {
        uint8_t _type = 0;
        uint8_t _ack = 0;
        uint8_t _stage = 0;
        uint8_t _quality = 0;
        uint16_t _ackquant = 0;
        uint8_t _alen = 0;
        uint16_t _dlen = 0;
        uint16_t _seqid = 0;
        uint8_t *_aptr = nullptr;
        char *_dptr = nullptr;

    public:
        uint8_t *addrptr() override
        {
            return _aptr;
        }
        uint8_t addrsize() override
        {
            return _alen;
        }

        char *dataptr() override
        {
            return _dptr;
        }
        uint16_t datasize() override
        {
            return _dlen;
        }

        char *endptr() override
        {
            return nullptr;
        }
        uint8_t type() override
        {
            return _type;
        }
        uint16_t seqid() override
        {
            return _seqid;
        }

        uint8_t *stageptr() override
        {
            return addrptr() + stage();
        }

        uint16_t full_length() override
        {
            return 0;
        }
        uint8_t quality() override
        {
            return _quality;
        }
        uint16_t ackquant() override
        {
            return _ackquant;
        }
        uint8_t stage() override
        {
            return _stage;
        }
        uint8_t ack() override
        {
            return _ack;
        }

        void set_addrsize(uint8_t arg) override
        {
            _alen = arg;
        }
        void set_datasize(uint16_t arg) override
        {
            _dlen = arg;
        }
        void set_type(uint8_t arg) override
        {
            _type = arg;
        }
        void set_quality(uint8_t arg) override
        {
            _quality = arg;
        }
        void set_ackquant(uint16_t arg) override
        {
            _ackquant = arg;
        }
        void set_stage(uint8_t arg) override
        {
            _stage = arg;
        }
        void set_seqid(uint16_t arg) override
        {
            _seqid = arg;
        }
        void set_ack(uint8_t arg) override
        {
            _ack = arg;
        }

        void invalidate() override
        {
            free(_aptr);
            _aptr = NULL;
        }

        void allocate_buffer(int alen, int dlen)
        {
            _alen = alen;
            _dlen = dlen;
            if (_aptr)
                invalidate();
            _aptr = (uint8_t *)malloc(alen + dlen + 1);
            _dptr = (char *)(_aptr + alen);
            *(_dptr + dlen) = 0;
        }

        void self_init() override {}

        ~morph_packet()
        {
            invalidate();
        }
    };

    template <class Header> class compacted_packet : public packet
    {
    public:
        Header _header = {};

    public:
        Header &header()
        {
            return _header;
        }

        uint8_t *addrptr()
        {
            return (uint8_t *)(&header() + 1);
        }

        uint8_t addrsize()
        {
            return _header.alen;
        }

        char *dataptr()
        {
            return (char *)(addrptr() + addrsize());
        }

        uint16_t datasize()
        {
            return (uint16_t)(full_length() - addrsize() -
                              sizeof(struct crow::header_v1));
        }

        char *endptr()
        {
            return (char *)&header() + full_length();
        }

        uint8_t type() override
        {
            return _header.u.f.type;
        }
        uint16_t seqid() override
        {
            return _header.seqid;
        }

        uint8_t *stageptr() override
        {
            return (uint8_t *)(&_header + 1) + _header.stg;
        }

        uint16_t full_length() override
        {
            return _header.flen;
        }

        uint8_t quality() override
        {
            return _header.qos();
        }

        uint16_t ackquant() override
        {
            return _header.ackquant();
        }

        uint8_t stage() override
        {
            return _header.stg;
        }

        uint8_t ack() override
        {
            return _header.u.f.ack;
        }

        void set_type(uint8_t arg) override
        {
            _header.u.f.type = arg;
        }

        void set_quality(uint8_t arg) override
        {
            _header.set_qos(arg);
        }
        void set_ackquant(uint16_t arg) override
        {
            _header.set_ackquant(arg);
        }

        void set_stage(uint8_t arg) override
        {
            _header.stg = arg;
        }

        void set_seqid(uint16_t arg) override
        {
            _header.seqid = arg;
        }

        void set_ack(uint8_t arg) override
        {
            _header.u.f.ack = arg;
        };

        void set_addrsize(uint8_t arg) override
        {
            (void)arg;
        }

        void set_datasize(uint16_t arg) override
        {
            (void)arg;
        }

        void self_init() override
        {
            *((char *)(&header()) + full_length()) = 0;
        }

        void invalidate() override
        {
            // pass
        }
    };

    /**
     * Выделить память для пакета.
     *
     * Выделяет adlen + sizeof(crow::packet) байт
     * @param adlen Суммарная длина адреса и данных в выделяемом пакете.
     */
    crow::packet *allocate_packet(int alen, int dlen);
    void deallocate_compacted_packet(crow::packet *pack);
    void deallocate_packet(crow::packet *pack);
    void packet_initialization(crow::packet *pack, crow::gateway *ingate);
    crow::packet *
    create_packet(crow::gateway *ingate, uint8_t addrsize, size_t datasize);

    template <class Header>
    crow::compacted_packet<Header> *allocate_compacted_packet(int alen,
                                                              int dlen)
    {
        return allocate_compacted_packet<Header>(alen + dlen);
    }

    template <class Header>
    crow::compacted_packet<Header> *allocate_compacted_packet(int adlen)
    {
        auto buflen = sizeof(crow::compacted_packet<Header>) + adlen + 1;
        system_lock();
        void *ret = malloc(buflen);
        memset(ret, 0, buflen);
        auto *pack = new (ret) crow::compacted_packet<Header>;

        if (ret)
            crow_allocated_count++;

        pack->set_destructor(crow::deallocate_compacted_packet);
        system_unlock();

        return pack;
    }

    template <class Header> Header extract_header(crow::packet *pack)
    {
        static_assert(std::is_same_v<Header, crow::header_v1> ||
                      std::is_same_v<Header, crow::header_v0>);

        if constexpr (std::is_same<Header, crow::header_v1>::value)
        {
            return pack->extract_header_v1();
        }
        else if constexpr (std::is_same<Header, crow::header_v0>::value)
        {
            return pack->extract_header_v0();
        }
    }

    // Только для аллокации через pool.
    void engage_packet_pool(void *zone, size_t zonesize, size_t elsize);
    igris::pool *get_package_pool();

    bool has_allocated();

    void diagnostic(const char *label, crow::packet *pack);
}

#endif
