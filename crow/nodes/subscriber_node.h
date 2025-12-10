#ifndef CROW_SUBSCRIBER_NODE_H
#define CROW_SUBSCRIBER_NODE_H

#include <crow/nodes/publisher_node.h>
#include <crow/proto/node.h>
#include <functional>
#include <igris/event/delegate.h>
#include <map>
#include <nos/print.h>
#include <vector>

namespace crow
{
    // Chunked reply constants (shared with service_node.h)
    inline constexpr uint8_t CHUNKED_REPLY_MARKER = 0x01;
    inline constexpr uint8_t CHUNK_FLAG_HAS_MORE = 0x01;

    class abstract_subscriber_node : public crow::publisher_node,
                                     public crow::alived_object
    {
    protected:
        int rqos = 0;
        int rackquant = 0;

    public:
        abstract_subscriber_node(crow::hostaddr_view addr,
                                 int node,
                                 nos::buffer theme)
            : publisher_node(addr, node, theme)
        {
        }

        abstract_subscriber_node(crow::hostaddr_view addr, nos::buffer theme)
            : publisher_node(addr, theme)
        {
        }

        abstract_subscriber_node() = default;
        void
        set_brocker_address(crow::hostaddr_view crowker_addr,
                            int crowker_node = CROWKER_SERVICE_BROCKER_NODE_NO);

        void set_rqos(int rqos, int rackquant)
        {
            this->rqos = rqos;
            this->rackquant = rackquant;
        }

        void subscribe();
        void subscribe_v2(bool updates, uint32_t get_latest);
        void unsubscribe();

        void init_subscribe(crow::hostaddr_view crowker_addr,
                            int crowker_node,
                            nos::buffer theme,
                            uint8_t qos,
                            uint16_t ackquant,
                            uint8_t rqos,
                            uint16_t rackquant);

        void init_subscribe(crow::hostaddr_view crowker_addr,
                            nos::buffer theme,
                            uint8_t qos,
                            uint16_t ackquant,
                            uint8_t rqos,
                            uint16_t rackquant)
        {
            init_subscribe(crowker_addr, CROWKER_SERVICE_BROCKER_NODE_NO, theme,
                           qos, ackquant, rqos, rackquant);
        }

        void keepalive_handle() override
        {
            subscribe();
        }
    };

    class subscriber_node : public crow::abstract_subscriber_node
    {
        using function = std::function<void(nos::buffer)>;
        function incoming_handler = {};

        // Chunked message reassembly
        std::map<uint16_t, std::vector<char>> _chunk_buffer;
        uint16_t _expected_chunks = 0;
        bool _receiving_chunks = false;

    public:
        subscriber_node() = default;
        subscriber_node(function incoming);
        subscriber_node(crow::hostaddr_view crowker_addr,
                        nos::buffer theme,
                        function incoming)
            : abstract_subscriber_node(crowker_addr, theme),
              incoming_handler(incoming)
        {
        }

        void set_handle(function incoming)
        {
            incoming_handler = incoming;
        }

        /// Handle incoming message with chunked reply support.
        /// Public for testing purposes.
        void handle_incoming_message(nos::buffer message);

    private:
        void incoming_packet(crow::packet *) override;
        void reset_chunk_buffer();
        bool try_reassemble_chunks(std::vector<char> &result);
    };
}

#endif