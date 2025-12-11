#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/subscriber_node.h>
#include <cstring>
#include <igris/time/systime.h>

void crow::subscriber_node::reset_chunk_buffer()
{
    _chunk_buffer.clear();
    _expected_chunks = 0;
    _receiving_chunks = false;
    _chunk_start_time = 0;
}

bool crow::subscriber_node::try_reassemble_chunks(std::vector<char> &result)
{
    for (uint16_t i = 0; i < _expected_chunks; ++i)
    {
        if (_chunk_buffer.find(i) == _chunk_buffer.end())
            return false;
    }

    result.clear();
    for (uint16_t i = 0; i < _expected_chunks; ++i)
    {
        auto &chunk = _chunk_buffer[i];
        result.insert(result.end(), chunk.begin(), chunk.end());
    }
    return true;
}

void crow::subscriber_node::handle_incoming_message(nos::buffer message)
{
    if (message.size() >= 4 &&
        static_cast<uint8_t>(message.data()[0]) == CHUNKED_REPLY_MARKER)
    {
        uint16_t chunk_id = static_cast<uint8_t>(message.data()[1]) |
                           (static_cast<uint8_t>(message.data()[2]) << 8);
        uint8_t flags = static_cast<uint8_t>(message.data()[3]);
        bool has_more = (flags & CHUNK_FLAG_HAS_MORE) != 0;

        // Check chunk_id limit to prevent memory exhaustion
        if (chunk_id >= MAX_CHUNKS)
        {
            reset_chunk_buffer();
            return;
        }

        // Check timeout for reassembly
        uint64_t now = igris::millis();
        if (_receiving_chunks && _chunk_start_time > 0 &&
            (now - _chunk_start_time) > CHUNK_REASSEMBLY_TIMEOUT_MS)
        {
            reset_chunk_buffer();
        }

        // Start timer on first chunk
        if (!_receiving_chunks)
        {
            _chunk_start_time = now;
        }

        std::vector<char> payload(message.data() + 4, message.data() + message.size());
        _chunk_buffer[chunk_id] = std::move(payload);
        _receiving_chunks = true;

        if (!has_more)
        {
            // This is the last chunk - now we know total count
            _expected_chunks = chunk_id + 1;
        }

        // Try to reassemble if we know how many chunks to expect
        if (_expected_chunks > 0)
        {
            std::vector<char> assembled;
            if (try_reassemble_chunks(assembled))
            {
                incoming_handler({assembled.data(), assembled.size()});
                reset_chunk_buffer();
            }
        }
    }
    else
    {
        if (_receiving_chunks)
        {
            reset_chunk_buffer();
        }
        incoming_handler(message);
    }
}

void crow::subscriber_node::incoming_packet(crow::packet *pack)
{
    auto &s = pack->subheader<pubsub_subheader>();

    switch (s.type)
    {
        case PubSubTypes::Consume:
        {
            auto &sh = pack->subheader<consume_subheader>();
            handle_incoming_message(sh.message());
        };
        break;

        default:
            break;
    }

    _tower->release(pack);
}

crow::subscriber_node::subscriber_node(function incoming)
    : incoming_handler(incoming)
{
}

void crow::abstract_subscriber_node::init_subscribe(
    crow::hostaddr_view crowker_addr,
    int crowker_node,
    nos::buffer theme,
    uint8_t qos,
    uint16_t ackquant,
    uint8_t rqos,
    uint16_t rackquant)
{
    this->crowker_addr = crowker_addr;
    this->crowker_node = crowker_node;
    this->theme = {theme.data(), theme.size()};
    this->qos = qos;
    this->ackquant = ackquant;
    this->rqos = rqos;
    this->rackquant = rackquant;
}

void crow::abstract_subscriber_node::set_brocker_address(
    crow::hostaddr_view crowker_addr, int crowker_node)
{
    this->crowker_addr = crowker_addr;
    this->crowker_node = crowker_node;
}

void crow::abstract_subscriber_node::subscribe()
{
    crow::subscribe_subheader sh;

    sh.type = PubSubTypes::Subscribe;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();

    const nos::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                sizeof(sh) - sizeof(node_subheader)},
                               theme};

    node::send_v(crowker_node, crowker_addr, iov, std::size(iov), qos,
                 ackquant);
}

void crow::abstract_subscriber_node::unsubscribe()
{
    crow::subscribe_subheader sh;

    sh.type = PubSubTypes::Unsubscribe;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();

    const nos::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                sizeof(sh) - sizeof(node_subheader)},
                               theme};

    node::send_v(crowker_node, crowker_addr, iov, std::size(iov), qos,
                 ackquant);
}

void crow::abstract_subscriber_node::subscribe_v2(bool updates,
                                                  uint32_t request_latest)
{
    crow::subscribe_subheader_v2 sh;

    sh.type = PubSubTypes::Subscribe_v2;
    sh.rqos = rqos;
    sh.rackquant = rackquant;
    sh.thmsize = theme.size();
    sh.cmd.f.subscribe_on_updates = updates;
    sh.cmd.f.request_latest = request_latest;

    const nos::buffer iov[] = {{(char *)&sh + sizeof(node_subheader),
                                sizeof(sh) - sizeof(node_subheader)},
                               theme};

    node::send_v(crowker_node, crowker_addr, iov, std::size(iov), qos,
                 ackquant);
}
