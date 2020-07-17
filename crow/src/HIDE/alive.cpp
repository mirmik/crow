#include <crow/alive.h>

#include <thread>
#include <chrono>

#include <igris/dprint.h>

bool ALIVE_THREAD_CANCEL_TOKEN = false;
std::thread ALIVE_THREAD;

void crow::start_alive(const std::vector<uint8_t>& addr, const char* netname, 
	uint16_t resend_time,  uint16_t dietime,
	uint8_t qos, uint16_t ackquant) 
{
	std::vector<uint8_t> vaddr = addr;
	std::string name = netname;

	ALIVE_THREAD = std::thread([=]()
	{
		while(1){
			if (ALIVE_THREAD_CANCEL_TOKEN) 
				return;

			send_alive(igris::buffer(vaddr.data(), vaddr.size()), name.c_str(), dietime, qos, ackquant);
			std::this_thread::sleep_for(std::chrono::milliseconds(resend_time));
		};
	});
}

void crow::stop_alive() 
{
	ALIVE_THREAD_CANCEL_TOKEN = true;
	ALIVE_THREAD.join();
}

void crow::alive_node::incoming_packet(crow::packet *pack) 
{
	auto data = crow::node_data(pack);
	struct alive_head * head = (struct alive_head *) data.data(); 
	const char* name = (const char*)(head + 1);

	std::vector<uint8_t> addr(pack->addrsize());
	memcpy(addr.data(), pack->addrptr(), pack->addrsize());

	std::chrono::milliseconds ms = 
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());

	auto rec = record { ms, std::string(name, head->netname_length) };
	dict[addr] = rec;
}