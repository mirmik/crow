#ifndef CROW_BROCKER_CLIENT_H
#define CROW_BROCKER_CLIENT_H

namespace crow
{
    class brocker_client
    {
        virtual void send_service_request(igris::buffer data);
        virtual void send_reply(igris::buffer data);

        virtual void send_message_to_subscriber(igris::buffer data);
        virtual void send_
    };

    class crow_node_client : public brocker_client
    {
        crow::hostaddr requestor_host;
        int requestor_node;
        int qos;
        int ackquant;
    };
}

#endif
