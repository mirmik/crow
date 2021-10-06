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
        PublishAndSubscribe
    };

    struct pubsub_subheader : public node_subheader
    {
        PubSubTypes type;
    };

    struct publish_subheader : public pubsub_subheader
    {
        int thmsize;
        int datsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
        igris::buffer message()
        {
            return {(char *)(this + 1) + thmsize, datsize};
        }
    };

    struct subscribe_subheader : public pubsub_subheader
    {
        int rqos;
        int rackquant;
        int keepalive;
        int thmsize;
        igris::buffer theme() { return {(char *)(this + 1), thmsize}; }
    };

    struct consume_subheader : public pubsub_subheader
    {
        int datsize;
        igris::buffer message() { return {(char *)(this + 1), datsize}; }
    };

    struct publish_and_subscribe_subheader : public pubsub_subheader
    {
    };
}

#endif