#include <crow/protocol.h>
#include <crow/tower.h>

void crow::protocol::enable() 
{
	dlist_add_tail(&lnk, &crow::protocols);
}