#include <crow/proto/node.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <thread>
#include <chrono>

#include <crow/proto/rpc.h>
#include <crow/address.h>

#include <nos/print.h>

crow::rpc_node rpcnode;

std::vector<uint8_t> crowaddr = crow::address("");

void mainfunc() 
{
	crow::rpc_requestor requestor(crowaddr);

	float result = 0;
	int status = requestor.request<float, int, float>("hello", result, 23, 52.7);	

	nos::println("Status:", status);
	nos::println("Result:", result);

	crow::stop_spin();
}

float hello(int a, float b) 
{
	return a + b;
}

int main()
{
	rpcnode.bind();
	rpcnode.add_delegate("hello", igris::make_delegate(hello));

	crow::diagnostic_enable();

	std::thread thr(mainfunc);

	crow::start_spin();
	crow::join_spin();

	thr.join();
}