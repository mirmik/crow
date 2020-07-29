#include <crow/proto/service.h>
#include <crow/gates/udpgate.h>

void add_service_handler(crow::service_packet_ptr ptr) 
{
	auto message = ptr.message();

	int32_t a = *(int32_t*)(message.data());
	int32_t b = *(int32_t*)(message.data() + sizeof(int32_t));
	int32_t c = a + b;

	ptr.answer({&c, sizeof(c)});
}

int main()
{
	auto add_service = crow::service("add", add_service_handler);

	crow::start_spin();
	while(1);
}