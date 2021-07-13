/**
@file
*/

#include <crow/gates/chardev_gateway.h>
#include <crow/tower.h>

void crow::chardev_gateway::send(struct crow::packet *pack)
{
	protocol->send(pack);
}

void crow::chardev_gateway::finish()
{
}

void crow::chardev_gateway::nblock_onestep()
{
	driver->nonblock_tryread();
}

crow::chardev_gateway::chardev_gateway(
	crow::chardev_driver * driver, 
	crow::chardev_protocol * protocol) 
{
	this->driver = driver;
	this->protocol = protocol;
}
