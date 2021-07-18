#include <crow/proto/msgbox.h>

crow::node_packet_ptr crow::msgbox::query(uint16_t rid,
                                          const crow::hostaddr_view &addr,
                                          const igris::buffer data, uint8_t qos,
                                          uint16_t ackquant)
{
    assert(dlist_empty(&messages));
    auto ptr = send(rid, addr, data, qos, ackquant);
    return receive();
}

crow::node_packet_ptr crow::msgbox::receive()
{
    sem_down(&message_lock);

    while (dlist_empty(&messages))
    {
        sem_up(&message_lock);
        // Тут должна быть проверка на то что сообщение не пришло,
        // пока мы были между отпущенным семафором и waitevent
        // тоесть, нужен специальный вариант waitevent
        int sts = waitevent();
        if (sts == -1)
            return nullptr;
        sem_down(&message_lock);
    }

    crow::packet *pack = dlist_first_entry(&messages, crow::packet, ulnk);
    dlist_del_init(&pack->ulnk);

    sem_up(&message_lock);

    return pack;
}

crow::packet_ptr crow::msgbox::reply(crow::node_packet_ptr msg,
                                     igris::buffer data, uint8_t qos,
                                     uint16_t ackquant)
{
    return send(msg.rid(), msg->addr(), data, qos, ackquant);
}

void crow::msgbox::incoming_packet(crow::packet *pack)
{
    sem_down(&message_lock);
    dlist_add_tail(&pack->ulnk, &messages);
    sem_up(&message_lock);

    notify_one(0);
}

void crow::msgbox::undelivered_packet(crow::packet *pack) { notify_one(-1); }

crow::msgbox::~msgbox()
{
    sem_down(&message_lock);
    while (!dlist_empty(&messages))
    {
        crow::packet *pack = dlist_first_entry(&messages, crow::packet, ulnk);
        dlist_del_init(&pack->ulnk);
        crow::release(pack);
    }
    sem_up(&message_lock);
}
