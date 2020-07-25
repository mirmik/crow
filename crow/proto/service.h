/**
	Протокол для брокера на основе механизма нодов и почтового ящика.
	Позволяет зарегистрировать сервис в брокере, передавать команды и получать ответы.
*/

#include <crow/proto/msgbox.h>

#define CROW_SERVICE_REGISTER 1
#define CROW_BROCKER_SERVICE_NODE 5

namespace crow
{
	struct service_to_brocker_register
	{
		uint8_t  type = 0;
		uint16_t namelen;
		char*    name;
	} __attribute__((packed));

	struct from_user_to_brocker_question
	{
		uint8_t type = 1;
		int16_t  mark;
		uint16_t namelen;
		uint16_t datalen;
		uint8_t* name;
		uint8_t* data;
	} __attribute__((packed));

	struct from_brocker_to_service_question
	{
		uint8_t type = 2;
		uint32_t brocker_mark;
		uint16_t datalen;
		uint8_t* data;
	} __attribute__((packed));

	struct from_service_to_brocker_answer
	{
		uint8_t type = 3;
		uint32_t brocker_mark;
		uint16_t datalen;
		uint8_t* data;
	} __attribute__((packed));

	struct from_brocker_to_user_answer
	{
		uint8_t type = 4;
		int16_t  mark;
		uint16_t datalen;
		uint8_t* data;
	} __attribute__((packed));

	class service : public crow::node
	{
	private:
		crow::hostaddr brocker_addr;
		const char * _name;

	public:
		service(const char * name) : _name(name)
		{}

		const char * name() { return _name; }

		virtual void doit(crow::packet * pack);

		void incoming_packet(crow::packet *pack) override
		{

		};

		void undelivered_packet(crow::packet *pack) override
		{

		};

		void dial(const crow::hostaddr & addr,
		          int qos,
		          uint16_t ackquant)
		{
			brocker_addr = addr;

			service_to_brocker_register subheader;

			uint16_t namelen = strlen(name());

			igris::buffer data[3] =
			{
				{ &subheader.type, sizeof(subheader.type) },
				{ &namelen, sizeof(namelen) },
				{ name(), namelen },
			};

			send_v(
			    CROW_BROCKER_SERVICE_NODE,
			    addr,
			    data,
			    std::size(data),
			    qos,
			    ackquant);
		}
	};
}