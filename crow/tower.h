/** @file */

#ifndef CROW_TOWER_H
#define CROW_TOWER_H

#include <crow/defs.h>
#include <crow/gateway.h>
#include <crow/packet_ptr.h>
#include <igris/event/delegate.h>
#include <nos/buffer.h>
#include <string>

#define CROW_SENDED 0
#define CROW_WRONG_ADDRESS -1

// Forward declaration of Tower class
namespace crow
{
    class Tower;
    Tower &default_tower();
}

namespace crow
{
    // Configuration accessors (delegate to default_tower())
    void set_diagnostic_noack(bool value);
    bool get_diagnostic_noack();

    void set_debug_data_size(uint16_t size);
    uint16_t get_debug_data_size();

    void set_retransling_allowed(bool value);
    bool get_retransling_allowed();

    unsigned int get_total_travelled();

    // Handler accessors (delegate to default_tower())
    void set_default_incoming_handler(void (*handler)(packet *));
    void set_undelivered_handler(void (*handler)(packet *));
    void set_unsleep_handler(igris::delegate<void> handler);

    // Передать пакет в обработку.
    packet_ptr travel(crow::packet *pack);
    void nocontrol_travel(crow::packet *pack, bool fastsend);

    // Включить трассировку пакетов.
    void enable_diagnostic();
    bool diagnostic_enabled();

    // Включить диагностику прохождения, жизни пакетов
    void diagnostic_setup(bool en);

    // Отправить пакет.
    crow::packet_ptr send(const crow::hostaddr_view &addr,
                          nos::buffer data,
                          uint8_t type,
                          uint8_t qos,
                          uint16_t ackquant,
                          bool async = false);

    crow::packet_ptr send_v(const crow::hostaddr_view &addr,
                            const nos::buffer *vec,
                            size_t veclen,
                            uint8_t type,
                            uint8_t qos,
                            uint16_t ackquant,
                            bool async = false);

    crow::packet_ptr send_vv(const crow::hostaddr_view &addr,
                             const nos::buffer *vec,
                             size_t veclen,
                             const nos::buffer *vec2,
                             size_t veclen2,
                             uint8_t type,
                             uint8_t qos,
                             uint16_t ackquant,
                             bool async = false);

    crow::packet_ptr send_vvv(const crow::hostaddr_view &addr,
                              const nos::buffer *vec,
                              size_t veclen,
                              const nos::buffer *vec2,
                              size_t veclen2,
                              const nos::buffer *vec3,
                              size_t veclen3,
                              uint8_t type,
                              uint8_t qos,
                              uint16_t ackquant,
                              bool async = false);

    // Эта функция вызывается вратами после обработки отсылаемого пакета.
    void return_to_tower(crow::packet *pack, uint8_t sts);

    // Используется пользовательским кодом для освобождения пакета.
    void release(crow::packet *pack);
    void utilize(crow::packet *pack);

    void onestep();
    void onestep_travel_only();

    void spin();
    void spin_with_select();

    int stop_spin(bool wait = true);
    int start_spin_with_select();
    int start_spin_without_select();
    int start_spin();

    [[deprecated]] void spin_join();
    void join_spin();

    bool has_untravelled();
    bool has_untravelled_now();
    void print_list_counts();

    bool fully_empty();

    int64_t get_minimal_timeout();

    // Завершить гейты.
    void finish();

    // Очистить состояние для тестов
    void reset_for_test();

    int incomming_stage_count();
    int outers_stage_count();
    void unsleep();

    void set_spin_cancel_token();

    void spin_with_select_realtime(int abort_on_fault);
    int start_spin_with_select_realtime(int abort_on_fault);
    int start_spin_realtime(int abort_on_fault);

    void tower_release(crow::packet *pack);

    // TIME_WAIT duration for QOS2 duplicate filtering (default: 15000 ms)
    void set_time_wait_duration(uint32_t duration_ms);
    uint32_t get_time_wait_duration();

    // Set initial sequence number (useful for random seqid at startup)
    void set_initial_seqid(uint16_t seqid);
    uint16_t get_current_seqid();

    // Application name for diagnostic messages
    void set_diagnostic_label(const std::string &name);
    const std::string &get_diagnostic_label();
}

#endif
