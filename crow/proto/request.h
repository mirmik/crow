#ifndef CROW_REQUEST_API_H
#define CROW_REQUEST_API_H

#include <crow/proto/node.h>

namespace crow 
{
	int sync_query(
		uint8_t* addr, uint8_t alen, int sid, int rid, 
		const void * data, uint16_t dlen) 
	{
		auto tempnode = crow::node();
	}
}

#endif