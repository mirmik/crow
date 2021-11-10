/**
@file
*/

#include <crow/gates/chardev_gateway.h>
#include <crow/tower.h>

void crow::chardev_gateway::send(crow::packet *pack)
{
    system_lock();

    if (dlist_empty(&to_send))
    {
        dosend(pack);
    }
    else
    {
        dlist_move_tail(&pack->lnk, &to_send);
    }

    system_unlock();
}

void crow::chardev_gateway::dosend(crow::packet *pack)
{
    insend = pack;
    protocol->send_automate_reset(insend);
    driver->start_send();
}

void crow::chardev_gateway::init_recv()
{
    system_lock();
    rpack = crow_allocate_packet(packet_dataaddr_size);
    memset((void *)rpack, 0,
           packet_dataaddr_size + sizeof(crow::header_v1));
    if (rpack == nullptr)
    {
        return;
        system_unlock();
    }

    protocol->receive_automate_setbuf((char *)&rpack->header(),
                                      packet_dataaddr_size);
    protocol->receive_automate_reset();

    system_unlock();
}

void crow::chardev_gateway::nblock_onestep()
{

    if (rpack == nullptr)
        init_recv();

    driver->nonblock_tryread();

    /*while (driver->ready_for_recv())
    {
        char c;
        driver->read(&c, 1);

        int sts = protocol->receive_automate_newdata(c);

        //if (sts == CROW_NEWPACK)
        //{
        //nos::println("newpack");
        //}
    }*/
}

void crow::chardev_gateway::newline_handler()
{
    crow::compacted_packet *block = rpack;
    rpack = NULL;

    block->revert_gate(id);

    crow_packet_initialization(block, this);
    crow::nocontrol_travel(block, false);
}
crow::chardev_gateway::chardev_gateway(crow::chardev_driver *driver,
                                       crow::chardev_protocol *protocol)
{
    this->driver = driver;
    this->protocol = protocol;
}

void crow::chardev_gateway::newdata(char c)
{
    protocol->receive_automate_newdata(c);
}

void crow::chardev_gateway::packet_sended_handler() {}