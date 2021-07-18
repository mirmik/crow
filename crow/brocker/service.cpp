/** @file */

#include <crow/brocker/service.h>

crow::crowker_service_control_node_cls *crow::crowker_service_control_node()
{
    static crow::crowker_service_control_node_cls *ptr =
        new crowker_service_control_node_cls();
    return ptr;
}

void crow::async_request_callback(void *arg, int sts, crow_packet *pack)
{
    service_record *cbrec = (service_record *)arg;
    crowker_service_control_node()->send(cbrec->naddr.nid, cbrec->naddr.naddr,
                                         node::message(pack), pack->header.qos,
                                         pack->header.ackquant);

    delete cbrec;
}
