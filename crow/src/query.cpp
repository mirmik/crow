#include <crow/query.h>
#include <crow/tower.h>

crow::query_function * crow::query_table;

void crow::query_protocol_handler(crow::packet * pack)
{
	igris::buffer raw, theme, data;
//	uint16_t qcount;

	crow::query_header* header;

	if (crow::query_table == nullptr)
		goto end_label;

	raw = pack->rawdata();
	header = (crow::query_header *) raw.data();

	theme = { raw.data() + sizeof(crow::query_header), header->thmsz };
	data = { raw.data() + sizeof(crow::query_header) + header->thmsz, header->datsz };

	for (const crow::query_function * q = crow::query_table; q->func != NULL; ++q)
	{
		if (theme == igris::buffer(q->name, strlen(q->name))) 
		{
			dprln("OUR CLIENT TODO");

			goto end_label;
		}
	}

	// Not Found
	crow::query_answer(pack, NULL, 0, CROW_QUERY_ENOENT);

end_label:
	crow::release(pack);
	return;
}

void crow::query_answer(crow::packet * pack,
                        const char* dat, size_t datsz, uint8_t errcode)
{
	crow::query_header out_header;
	crow::query_header * in_header;

	char * in_header_thm;

	in_header = (crow::query_header*) pack->rawdata().data();
	in_header_thm = (char*)(in_header + 1);

	out_header.thmsz = in_header->thmsz;
	out_header.datsz = datsz;
	out_header.qid = in_header->qid;
	out_header.code = errcode;

	struct iovec iov[] =
	{
		{&out_header, sizeof(query_header)},
		{(void *)in_header_thm, in_header->thmsz},
		{(void *)dat, datsz}
	};

	crow::send_v(pack->addrptr(), pack->addrsize(), iov, 3,
	             CROW_QUERY_PROTOCOL, pack->header.qos, pack->header.ackquant);

}
