/**
@file
*/

#include <crow/gates/chardev_gateway.h>
#include <crow/tower.h>

void crow::chardev_gateway::send(struct crow::packet *pack)
{
	driver->send(pack);
    crow::return_to_tower(pack, CROW_SENDED);
}

void crow::chardev_gateway::finish()
{
}

crow::chardev_gateway::chardev_gateway(
	crow::chardev_driver * driver, 
	crow::chardev_protocol * protocol) 
{
	this->driver = driver;
	this->protocol = protocol;

	this->driver->protocol = protocol;
}
