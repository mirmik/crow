#include <crow/brocker/service.h>
[]
crow::crowker_service_control_node_cls * crow::crowker_service_control_node()
{
	static crow::crowker_service_control_node_cls * ptr = new crowker_service_control_node_cls();
	return ptr;
}


void crow::crowker_service_callback(void* arg, int sts, crow::packet * pack)
{
	crowker_service_callback_record * cbrec = (crowker_service_callback_record *) arg;
	cbrec->service->send(
	{ cbrec->host.data(), cbrec->host.size() },
	cbrec->id,
	node::message(pack),
	pack->qos(),
	pack->ackquant()
	);

	delete cbrec;
}