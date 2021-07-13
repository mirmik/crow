/**
@file
*/

#include <crow/gates/chardev_gateway.h>
#include <crow/tower.h>

void crow::chardev_gateway::send(struct crow::packet *pack)
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

void crow::chardev_gateway::dosend(struct crow::packet *pack) 
{
	insend = pack;	
	protocol->send_automate_reset(insend);
	driver->start_send();
}

void crow::chardev_gateway::finish()
{
}

void crow::chardev_gateway::init_recv()
{
	system_lock();
	rpack = (struct crow::packet*) crow::allocate_packet(packet_dataaddr_size);
	memset((void*)rpack, 0, packet_dataaddr_size + sizeof(crow::packet::header));
	if (rpack == nullptr)
	{
		return;
		system_unlock();
	}

	protocol->receive_automate_setbuf((char*)&rpack->header, packet_dataaddr_size);
	protocol->receive_automate_reset();

	system_unlock();
}

void crow::chardev_gateway::nblock_onestep()
{

	if (rpack == nullptr)
		init_recv();

	while (driver->ready_for_recv())
	{
		char c;
		driver->read(&c, 1);

		int sts = protocol->receive_automate_newdata(c);

		//if (sts == CROW_NEWPACK)
		//{
			//nos::println("newpack");
		//}
	}
}

crow::chardev_gateway::chardev_gateway(
    crow::chardev_driver * driver,
    crow::chardev_protocol * protocol)
{
	this->driver = driver;
	this->protocol = protocol;
}
