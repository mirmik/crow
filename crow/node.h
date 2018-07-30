#ifndef G0_CORE_H
#define G0_CORE_H

#include <gxx/container/dlist.h>
#include <gxx/datastruct/iovec.h>
#include <crow/packet.h>

namespace crow {
	struct node {
		dlist_head lnk;
		uint16_t id;
		virtual void incoming_packet(crow::packet* pack) = 0;
		node() { dlist_init(&lnk); }
	};

	struct subheader {
		uint16_t sid;
		uint16_t rid;		
	} G1_PACKED;

	static inline subheader* get_subheader(crow::packet* pack) {
		return (subheader*) pack->dataptr();
	}

	static inline gxx::buffer get_datasect(crow::packet* pack) {
		return gxx::buffer(pack->dataptr() + sizeof(subheader), pack->datasize() - sizeof(subheader));
	}	

	extern gxx::dlist<crow::node, &crow::node::lnk> nodes;

	/// Добавить сервис к ядру.
	void link_node(crow::node* srvs, uint16_t id);
	void __node_send(uint16_t sid, uint16_t rid, const uint8_t* raddr, size_t rlen, const char* data, size_t size, crow::QoS qos);
}

#endif