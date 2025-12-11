#include <crow/nodes/pubsub_defs.h>
#include <crow/nodes/service_node.h>
#include <crow/warn.h>
#include <nos/print.h>
#include <cstring>

void crow::service_node::incoming_packet(crow::packet *pack)
{
    curpack = pack;
    auto &subheader = pack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    auto message = nos::buffer{data.data() + 1 + reply_theme_length,
                               data.size() - 1 - reply_theme_length};
    dlg(message.data(), message.size(), *this);

    // release after reply
    _tower->release(pack);
}

void crow::service_node::reply_single(const char *answ, size_t size)
{
    auto &subheader = curpack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    auto reply_theme = nos::buffer(data.data() + 1, reply_theme_length);
    if (reply_theme != "__noanswer__")
        publish(curpack->addr(), subheader.sid, reply_theme, {answ, size}, qos,
                ackquant);
}

void crow::service_node::reply_chunked(const char *answ, size_t size)
{
    auto &subheader = curpack->subheader<consume_subheader>();
    auto data = subheader.message();
    int reply_theme_length = data.data()[0];
    auto reply_theme = nos::buffer(data.data() + 1, reply_theme_length);

    if (reply_theme == "__noanswer__")
        return;

    // Chunk header: [marker:1][chunk_id:2][flags:1] = 4 bytes
    constexpr size_t CHUNK_HEADER_SIZE = 4;
    size_t payload_per_chunk = _chunk_size - CHUNK_HEADER_SIZE;

    // Static buffer on stack - max chunk size is typically 400-512 bytes
    // Use 512 as reasonable max for embedded systems
    char chunk_buf[512];
    if (_chunk_size > sizeof(chunk_buf))
    {
        // Chunk size too large for stack buffer, fall back to single reply
        reply_single(answ, size);
        return;
    }

    size_t offset = 0;
    uint16_t chunk_id = 0;

    while (offset < size)
    {
        size_t remaining = size - offset;
        size_t chunk_payload = (remaining > payload_per_chunk) ? payload_per_chunk : remaining;
        bool has_more = (offset + chunk_payload < size);

        // Build chunk: [marker][chunk_id_lo][chunk_id_hi][flags][payload...]
        chunk_buf[0] = static_cast<char>(CHUNKED_REPLY_MARKER);
        chunk_buf[1] = static_cast<char>(chunk_id & 0xFF);
        chunk_buf[2] = static_cast<char>((chunk_id >> 8) & 0xFF);
        chunk_buf[3] = has_more ? CHUNK_FLAG_HAS_MORE : 0;
        std::memcpy(chunk_buf + CHUNK_HEADER_SIZE, answ + offset, chunk_payload);

        // Use QOS=1 (TARGET_ACK) for all chunks to ensure delivery
        // QOS=0 can lose intermediate chunks breaking reassembly
        uint8_t chunk_qos = has_more ? 1 : qos;
        uint16_t chunk_ackquant = ackquant;

        publish(curpack->addr(), subheader.sid, reply_theme,
                {chunk_buf, CHUNK_HEADER_SIZE + chunk_payload}, chunk_qos, chunk_ackquant);

        offset += chunk_payload;
        chunk_id++;
    }
}

void crow::service_node::reply(const char *answ, size_t size)
{
    // If chunking disabled or data fits in single chunk, use legacy mode
    if (_chunk_size == 0 || size <= _chunk_size)
    {
        reply_single(answ, size);
    }
    else
    {
        reply_chunked(answ, size);
    }
}
