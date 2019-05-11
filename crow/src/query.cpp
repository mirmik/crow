#include <crow/query.h>

void crow::query_protocol_handler(crow::packet * pack) 
{
	igris::buffer raw, theme, data;
	uint8_t thmsz;
	uint16_t datsz;
	uint16_t qcount;
	
	if (crow::query_table == nullptr) 
	{
		crow::release(pack);
		return;
	}

	raw = pack->rawdata();
	
	memcpy(&thmsz,  raw.data() + 0, 								sizeof(thmsz));
	memcpy(&datsz,  raw.data() + sizeof(thmsz), 					sizeof(datsz));
	memcpy(&qcount, raw.data() + sizeof(thmsz) + sizeof(datsz), 	sizeof(qcount));

	theme = { raw.data() + sizeof(thmsz) + sizeof(datsz) + sizeof(qcount), 			thmsz };
	data = { raw.data() + sizeof(thmsz) + sizeof(datsz) + sizeof(qcount) + thmsz, 	datsz };

	for (const auto& q : crow::query_table) 
	{
		//if 
	}

	crow::query_answer_error
}