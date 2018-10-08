/**
	@file
*/

#ifndef G1_GATES_SERIAL_GSTUFF_H
#define G1_GATES_SERIAL_GSTUFF_H

#include <crow/tower.h>
#include <crow/gateway.h>

struct crow_serial_gstuff {
	struct crow_gw gw;
	struct crowket * rpack;

	struct gstuff_automate recver;
	struct gstuff_sender sender;

} crow_serial_gstuff;

__BEGIN_DECLS

//void crow_serial_gstuff_open(struct crow_serial_gstuff* gw, uint16_t port);
crow_gw_t* crow_create_serial_gstuff(const char* path, uint32_t baudrate, uint8_t id);

__END_DECLS

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


#endif