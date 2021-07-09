#include <crow/gates/chardev_protocol.h>

void crow::chardev_protocol::send(crow::packet * pack) 
{
	system_lock();
	dlist_move(pack->lnk, &queue);
	system_unlock();
}

void crow::chardev_protocol::newdata(const char * data, unsigned int size) 
{

}

void crow::chardev_protocol::onestep() 
{

}
