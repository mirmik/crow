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
    semaphore_down(&message_lock);

    while (dlist_empty(&messages))
    {
        semaphore_up(&message_lock);
        // Тут должна быть проверка на то что сообщение не пришло,
        // пока мы были между отпущенным семафором и waitevent
        // тоесть, нужен специальный вариант waitevent
        int sts = waitevent();
        if (sts == -1)
            return nullptr;
        semaphore_down(&message_lock);
    }

    crow::packet *pack = dlist_first_entry(&messages, crow::packet, ulnk);
    dlist_del_init(&pack->ulnk);

    semaphore_up(&message_lock);

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
    semaphore_down(&message_lock);
    dlist_add_tail(&pack->ulnk, &messages);
    semaphore_up(&message_lock);

    notify_one(0);
}

void crow::msgbox::undelivered_packet(crow::packet *pack) { notify_one(-1); }

crow::msgbox::~msgbox()
{
    semaphore_down(&message_lock);
    while (!dlist_empty(&messages))
    {
        crow::packet *pack = dlist_first_entry(&messages, crow::packet, ulnk);
        dlist_del_init(&pack->ulnk);
        crow::release(pack);
    }
    semaphore_up(&message_lock);
}
