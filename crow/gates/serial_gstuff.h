/**
	@file
*/

#ifndef G1_GATES_SERIAL_GSTUFF_H
#define G1_GATES_SERIAL_GSTUFF_H

#include <crow/tower.h>

/*#include <crow/gateway.h>
#include <gxx/gstuff/sender.h>
#include <gxx/gstuff/automate.h>

#include <gxx/serial/serial.h>
*/
/*#include <gxx/io/std.h>

#include <mutex>

namespace crow {
	std::mutex mtx;

	struct serial_gstuff_gate : public gateway {
		gxx::gstuff::automate recver;
		//gxx::gstuff::sender sender;

		crow::packet* rpack = nullptr;
		//gxx::io::iostream* strm;
		serial::Serial* ser;

		serial_gstuff_gate(serial::Serial* ser) : ser(ser) {
			recver.debug_mode(true);
			recver.set_callback(gxx::make_delegate(&serial_gstuff_gate::handler, this));
		}

		void send(crow::packet* pack) override {
			std::string str;
			gxx::io::std_string_writer strm(str);
			gxx::gstuff::sender sender(strm);

			sender.start_message();
			sender.write((char*)&pack->header, pack->header.flen);
			sender.end_message();

			mtx.lock();
			ser->write((uint8_t*)str.data(), str.size());
			mtx.unlock();

			crow::return_to_tower(pack, crow::status::Sended);
		}

		void nonblock_onestep() override {
			if (rpack == nullptr) {
				init_recv();
			}

			char c;
			//int len = read(ser->fd(), (uint8_t*)&c, 1);
			int len = ser->read((uint8_t*)&c, 1);
			if (len == 1) {
				//dprhex(c); dpr("\t"); gxx::println(gxx::dstring(&c, 1));
				recver.newchar(c);
			}
		}

		void init_recv() {
			rpack = (crow::packet*) malloc(128 + sizeof(crow::packet) - sizeof(crow::packet_header));
			recver.init(gxx::buffer((char*)&rpack->header, 128));
		}

		void handler(gxx::buffer) {
			crow::packet* block = rpack;
			init_recv();

			block->revert_stage(id);

			crow::packet_initialization(block, this);
			crow::travel(block);
		}
	};
}*/

__BEGIN_DECLS

void crow_serialgate_send(crow_gw_t* gw, crow_packet_t* pack);
void crow_serialgate_nblock_onestep(crow_gw_t* gw);

const crow_gw_operations crow_serialgate_ops = {
	.send = crow_serialgate_send,
	.nblock_onestep = crow_serialgate_nblock_onestep
};

typedef struct crow_serialgate {
	struct crow_gw gw;
	void* privdata;
} crow_serialgate_t;

void crow_serialgate_open(crow_serialgate_t* gw, uint16_t port);

crow_gw_t* crow_create_serialgate(const char* path, uint32_t baudrate, uint8_t id);/* {
	crow_serialgate_t* g = (crow_serialgate*) malloc(sizeof(crow_serialgate));
	crow_serialgate_open(g, port); // TODO: should return NULL on error
	crow_link_gate(&g->gw, id);
	return &g->gw;
}*/

__END_DECLS

#endif