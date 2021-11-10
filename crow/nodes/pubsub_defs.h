#ifndef CROW_PUBSUB_DEFS_H
#define CROW_PUBSUB_DEFS_H

#include <crow/proto/node.h>
#include <igris/buffer.h>

namespace crow
{
    enum class PubSubTypes : uint8_t
    {
        Publish,
        Subscribe,
        Consume,
        Request
    };

    struct pubsub_subheader : public node_subheader
    {
        PubSubTypes type;
    } __attribute__((packed));

    struct publish_subheader : public pubsub_subheader
    {
        uint8_t thmsize;
        uint16_t datsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
        igris::buffer message()
        {
            return {(char *)(this + 1) + thmsize, datsize};
        }
    } __attribute__((packed));

    struct subscribe_subheader : public pubsub_subheader
    {
        uint8_t rqos;
        uint16_t rackquant;
        uint8_t thmsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
    } __attribute__((packed));

    struct consume_subheader : public pubsub_subheader
    {
        uint8_t thmsize;
        uint16_t datsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
        igris::buffer message()
        {
            return {(char *)(this + 1) + thmsize, datsize};
        }
    } __attribute__((packed));

    struct request_subheader : public pubsub_subheader
    {
        uint8_t rqos;
        uint16_t rackquant;
        uint8_t thmsize;
        uint8_t repthmsize;
        uint16_t datsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
        igris::buffer reply_theme()
        {
            return {(char *)(this + 1) + thmsize, repthmsize};
        }
        igris::buffer message()
        {
            return {(char *)(this + 1) + thmsize + repthmsize, datsize};
        }
    } __attribute__((packed));

    struct publish_and_subscribe_subheader : public pubsub_subheader
    {
    } __attribute__((packed));
}

#endif