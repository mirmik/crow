#ifndef G1_GATES_SERIAL_GSTUFF_H
#define G1_GATES_SERIAL_GSTUFF_H

#include <crow/tower.h>
#include <crow/gateway.h>
#include <gxx/gstuff/sender.h>
#include <gxx/gstuff/automate.h>

namespace crow {
	struct serial_gstuff_gate : public gateway {
		gxx::gstuff::automate recver;
		gxx::gstuff::sender sender;

		crow::packet* rpack = nullptr;
		gxx::io::iostream* strm;


		serial_gstuff_gate(gxx::io::iostream* strm) : strm(strm), sender(*strm) {
			recver.debug_mode(false);
			recver.set_callback(gxx::make_delegate(&serial_gstuff_gate::handler, this));
		}

		void send(crow::packet* pack) override {
			sender.start_message();
			sender.write((char*)&pack->header, pack->header.flen);
			sender.end_message();
			crow::return_to_tower(pack, crow::status::Sended);
		}

		void nonblock_onestep() override {
			if (rpack == nullptr) {
				init_recv();
			}

			char c;
			int len = strm->read(&c, 1);
			if (len == 1) {
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
}

#endif