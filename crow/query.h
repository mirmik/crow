#ifndef CROW_QUERY_H
#define CROW_QUERY_H

#include <crow/packet.h>

namespace crow
{
	///////////////////////////////////////////////////
	struct query_function
	{
		const char * name;
		void(*func)(crow::packet* pack);
	};

	extern query_function * query_table;
	////////////////////////////////////////////////////

	struct query_header
	{
		uint8_t thmsz;
		uint16_t datsz;
		uint8_t qid;
		uint8_t errcode;
	} __attribute__((packed));


	void query_protocol_handler(crow::packet * pack);

	crow::packet * send_query(const void *raddr, size_t rlen,
	                          const char* func,
	                          const void * data, size_t dsize,
	                          uint8_t qos, uint16_t ackquant);


	void query_answer(crow::packet* pack,
	                  const char* dat, size_t datsz, uint8_t errcode);
}

#define CROW_QUERY_TBLFIN { NULL, NULL }

#endif