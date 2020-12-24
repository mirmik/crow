#include <crow/proto/node.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <thread>
#include <chrono>

#include <crow/proto/rpc.h>
#include <crow/address.h>

#include <crow/crow.h>

#include <nos/print.h>

crow::udpgate ugate;
crow::rpc_node rpcnode;

float hello(int a, float b) 
{
	return a + b;
}

int main()
{
	ugate.bind();
	ugate.open(10010);

	rpcnode.bind();
	rpcnode.add_delegate("hello", igris::make_delegate(hello));

	crow::diagnostic_enable();

	crow::start_spin();
	crow::join_spin();
}