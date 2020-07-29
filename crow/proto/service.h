/**
	Протокол для брокера на основе механизма нодов и почтового ящика.
	Позволяет зарегистрировать сервис в брокере, передавать команды и получать ответы.
*/

#include <crow/proto/msgbox.h>
#include <igris/event/delegate.h>
#include <igris/binreader.h>

#define CROW_SERVICE_REGISTER 1
#define CROW_BROCKER_SERVICE_NODE 5

#define CROW_SERVICE_SERVICE_TO_BROCKER_REGISTER 0
#define CROW_SERVICE_FROM_USER_TO_BROCKER_QUESTION 1
#define CROW_SERVICE_FROM_BROCKER_TO_SERVICE_QUESTION 2
#define CROW_SERVICE_FROM_SERVICE_TO_BROCKER_ANSWER 3
#define CROW_SERVICE_FROM_USER_TO_BROCKER_ANSWER 4

namespace crow
{
	class service;

	struct service_package_annotation
	{
		uint8_t type = 2;
		uint32_t second_mark;
		uint16_t datalen;
		uint16_t namelen;
		uint8_t* data;
		uint8_t* name;

		int parse(igris::buffer message)
		{
			igris::binreader reader;

			if (message.size() < 7)
				return -1;

			reader.init(message.data());
			
			reader.read_binary(type);
			reader.read_binary(second_mark);
			reader.read_binary(datalen);
			reader.read_binary(namelen);

			reader.bind_buffer(data, datalen);
			reader.bind_buffer(name, namelen);

			return 0;
		}

		void to_buffers(igris::buffer bufs[5])
		{
			bufs[0] = {&type, 1};
			bufs[1] = {&second_mark, sizeof(second_mark)};
			bufs[2] = {&datalen, sizeof(datalen)};
			bufs[3] = {&namelen, sizeof(namelen)};
			bufs[4] = {data, datalen};
			bufs[5] = {name, namelen};
		}
	};

	class service_packet_ptr : public node_packet_ptr
	{
	public:
		service_package_annotation & annot;
		crow::service * srv;

		service_packet_ptr(crow::packet *pack_,
		                   crow::service * srv,
		                   service_package_annotation & annotation)
			: node_packet_ptr(pack_), srv(srv), annot(annot)
		{}

		crow::service * service() { return srv; }
		igris::buffer   message() { return {annot.data, annot.datalen}; }

		packet_ptr answer(igris::buffer data);
	};

	class service : public crow::node
	{
		using dlg_t = igris::delegate <void, crow::service_packet_ptr>;

	private:
		crow::hostaddr brocker_addr;

		const char *   _name;
		dlg_t          _handler;

	public:
		service(const char * name, dlg_t dlg) : _name(name), _handler(dlg)
		{}

		const char * name() { return _name; }

		void incoming_packet(crow::packet *pack) override
		{
			int sts;
			service_package_annotation annot;
			igris::buffer message;

			message = node::message(pack);

			if ((sts = annot.parse(message)))
			{
				crow::release(pack);
				return;
			}

			_handler({ pack, this, annot });
		};

		void undelivered_packet(crow::packet *pack) override
		{
			crow::release(pack);
		};

		void dial(const crow::hostaddr & addr,
		          int qos,
		          uint16_t ackquant)
		{
			brocker_addr = addr;

			service_package_annotation annot;
			igris::buffer buffers[5];

			annot.to_buffers(buffers);

			node::send_v(
			    CROW_BROCKER_SERVICE_NODE,
			    addr,
			    buffers,
			    std::size(buffers),
			    qos,
			    ackquant);
		}
	};
}