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

double func_add(double a, double b) 
{
	return a + b;
}

double func_sub(double a, double b) 
{
	return a - b;
}

double func_mul(double a, double b) 
{
	return a * b;
}

double func_div(double a, double b) 
{
	return a / b;
}

int main()
{
	ugate.bind();
	ugate.open(10010);

	rpcnode.bind();
	rpcnode.add_delegate("add", igris::make_delegate(func_add));
	rpcnode.add_delegate("sub", igris::make_delegate(func_sub));
	rpcnode.add_delegate("mul", igris::make_delegate(func_mul));
	rpcnode.add_delegate("div", igris::make_delegate(func_div));

	crow::diagnostic_enable();

	crow::start_spin();
	crow::join_spin();
}