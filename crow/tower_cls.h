/** @file */

#ifndef CROW_TOWER_CLS_H
#define CROW_TOWER_CLS_H

#include <crow/defs.h>
#include <crow/gateway.h>
#include <crow/packet_ptr.h>
#include <crow/src/packet_htable.h>
#include <igris/datastruct/dlist.h>
#include <igris/event/delegate.h>
#include <nos/buffer.h>
#include <string>

namespace crow
{
    // TIME_WAIT entry for QOS2 duplicate filtering
    struct time_wait_entry
    {
        dlist_head lnk;       // For linked list
        dlist_head hlnk;      // For hash table
        uint64_t expire_time; // When this entry expires (milliseconds)
        uint16_t seqid;
        uint8_t addrsize;
        uint8_t addr[32]; // Max address size
    };

    class Tower
    {
    public:
        static constexpr size_t MAX_OUTTERS = 128;
        static constexpr size_t MAX_INCOMING = 128;
        static constexpr size_t TIME_WAIT_HTABLE_SIZE = 64;
        static constexpr size_t MAX_TIME_WAIT_ENTRIES = 256;

    private:
        // Packet queues
        dlist_head _travelled = DLIST_HEAD_INIT(_travelled);
        dlist_head _incoming = DLIST_HEAD_INIT(_incoming);
        dlist_head _outters = DLIST_HEAD_INIT(_outters);

        // Hash tables for fast lookup
        packet_htable _outters_htable;
        packet_htable _incoming_htable;

        // Counters and limits
        size_t _outters_count = 0;
        size_t _incoming_count = 0;

        // TIME_WAIT mechanism for QOS2 duplicate filtering
        dlist_head _time_wait_list = DLIST_HEAD_INIT(_time_wait_list);
        dlist_head _time_wait_htable[TIME_WAIT_HTABLE_SIZE];
        dlist_head _time_wait_free_list = DLIST_HEAD_INIT(_time_wait_free_list);
        time_wait_entry _time_wait_pool[MAX_TIME_WAIT_ENTRIES];
        size_t _time_wait_count = 0;
        uint32_t _time_wait_duration_ms = 15000;
        bool _time_wait_initialized = false;

        // Sequence counter
        uint16_t _seqcounter = 0;

        // Diagnostic settings
        bool _diagnostic_enabled = false;
        std::string _diagnostic_label;

        // Gateway list for this tower
        dlist_head _gateway_list = DLIST_HEAD_INIT(_gateway_list);
        dlist_head _c_gateway_list = DLIST_HEAD_INIT(_c_gateway_list);

        // Internal methods
        void time_wait_init();
        size_t time_wait_hash(uint16_t seqid);
        void time_wait_cleanup_expired();
        bool time_wait_check(uint16_t seqid,
                             const uint8_t *addr,
                             uint8_t addrsize);
        void time_wait_add(uint16_t seqid,
                           const uint8_t *addr,
                           uint8_t addrsize);

        void do_travel(packet *pack);
        void send_to_gate_phase(packet *pack);
        void incoming_ack_phase(packet *pack);
        void incoming_handler(packet *pack);

        void send_ack(packet *pack);
        void send_ack2(packet *pack);
        static void revert_address(packet *pack);

        void confirmed_utilize_from_outers(packet *pack);
        void qos_release_from_incoming(packet *pack);

        bool add_to_incoming_list(packet *pack);
        bool add_to_outters_list(packet *pack);

        void onestep_send_stage();
        void onestep_outers_stage();
        void onestep_incoming_stage();
        void onestep_keepalive_stage();

        void undelivered(packet *pack);

        packet_ptr transport(packet *pack, bool async);

    private:
        // Configuration
        bool _diagnostic_noack = false;
        uint16_t _debug_data_size = 60;
        unsigned int _total_travelled = 0;
        bool _retransling_allowed = false;

        // Handlers
        void (*_default_incoming_handler)(packet *pack) = nullptr;
        void (*_undelivered_handler)(packet *pack) = nullptr;
        igris::delegate<void> _unsleep_handler;

        // Flags for reentrancy protection
        bool _in_incoming_handler = false;
        bool _in_undelivered_handler = false;

    public:
        Tower();
        ~Tower();

        // Non-copyable, non-movable
        Tower(const Tower &) = delete;
        Tower &operator=(const Tower &) = delete;
        Tower(Tower &&) = delete;
        Tower &operator=(Tower &&) = delete;

        // Core API
        packet_ptr travel(packet *pack);
        void nocontrol_travel(packet *pack, bool fastsend);
        void return_to_tower(packet *pack, uint8_t sts);

        // Send API
        packet_ptr send(const hostaddr_view &addr,
                        nos::buffer data,
                        uint8_t type,
                        uint8_t qos,
                        uint16_t ackquant,
                        bool async = false);

        packet_ptr send_v(const hostaddr_view &addr,
                          const nos::buffer *vec,
                          size_t veclen,
                          uint8_t type,
                          uint8_t qos,
                          uint16_t ackquant,
                          bool async = false);

        packet_ptr send_vv(const hostaddr_view &addr,
                           const nos::buffer *vec,
                           size_t veclen,
                           const nos::buffer *vec2,
                           size_t veclen2,
                           uint8_t type,
                           uint8_t qos,
                           uint16_t ackquant,
                           bool async = false);

        packet_ptr send_vvv(const hostaddr_view &addr,
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

        // Packet management (static - don't depend on specific Tower instance)
        static void release(packet *pack);
        static void utilize(packet *pack);
        static void tower_release(packet *pack);

        // Event loop
        void onestep();
        void onestep_travel_only();

        // Status queries
        bool has_untravelled();
        bool has_untravelled_now();
        bool fully_empty();
        int64_t get_minimal_timeout();
        int incomming_stage_count();
        int outers_stage_count();
        void print_list_counts();

        // Diagnostic
        void enable_diagnostic();
        bool diagnostic_enabled() const;
        void diagnostic_setup(bool en);
        void set_diagnostic_label(const std::string &name);
        const std::string &get_diagnostic_label() const;

        // Configuration accessors
        void set_diagnostic_noack(bool value);
        bool get_diagnostic_noack() const;

        void set_debug_data_size(uint16_t size);
        uint16_t get_debug_data_size() const;

        void set_retransling_allowed(bool value);
        bool get_retransling_allowed() const;

        unsigned int get_total_travelled() const;
        void reset_total_travelled();

        // Handler accessors
        void set_default_incoming_handler(void (*handler)(packet *));
        void (*get_default_incoming_handler() const)(packet *);

        void set_undelivered_handler(void (*handler)(packet *));
        void (*get_undelivered_handler() const)(packet *);

        void set_unsleep_handler(igris::delegate<void> handler);
        igris::delegate<void> get_unsleep_handler() const;

        // TIME_WAIT configuration
        void set_time_wait_duration(uint32_t duration_ms);
        uint32_t get_time_wait_duration() const;

        // Sequence ID management
        void set_initial_seqid(uint16_t seqid);
        uint16_t get_current_seqid() const;

        // Gateway management
        dlist_head &gateway_list();
        dlist_head &c_gateway_list();
        gateway *get_gateway(int no);
        crow_gateway *get_c_gateway(int no);
        int bind_gateway(gateway *gate, int gateno);
        int bind_c_gateway(crow_gateway *gate, int gateno);

        // Reset for testing
        void reset_for_test();

        // Finish all gateways
        void finish();

        // Unsleep notification
        void unsleep();
    };

    // Get the default tower instance (lazy-initialized singleton)
    Tower &default_tower();

} // namespace crow

#endif
