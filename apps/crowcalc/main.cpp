#include <crow/proto/node.h>
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <thread>
#include <chrono>

#include <crow/proto/rpc.h>
#include <crow/nodes/service_node.h>
#include <crow/address.h>
#include <crow/crow.h>

#include <nos/fprint.h>
#include <igris/getopt/cliopts.h>

crow::udpgate ugate;
crow::rpc_node rpcnode;
crow::service_node service;

std::string hello(std::string inpstr) 
{
	return nos::format("HelloWorld: {}", inpstr);
}

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

int foo(char *, int, char * ans, int) 
{
	nos::println("foo");
	sprintf(ans, "HelloWorld\r\n");
	return strlen(ans);
}

int main(int argc, char ** argv)
{
	igris::cliopts cliopts;
	cliopts.add_option("debug", 'd');
	cliopts.add_integer("udp", 'u', 10020);
	cliopts.parse(argc, argv);
	bool debug = cliopts.get_option("debug");

	if (debug)
		crow::diagnostic_setup(true);

	if (argc == 1) 
	{
		nos::println("Usage: crowcalc CROWKER");
		exit(0);
	}

	crow::hostaddr crowker = crow::address(argv[1]);
	nos::println("Crowker: ", argv[1]);

	ugate.bind();

	service.bind();
	service.set_handle(foo); 
	service.subscribe(crowker, CROWKER_SERVICE_BROCKER_NODE_NO, "calc", 2,50,2,50);

	int port = cliopts.get_integer("udp");
	nos::fprintln("Make udp server, port : {}", port);
	ugate.open(port);

	nos::println("Use node :", CROW_RPC_NODE_NO);
	rpcnode.bind();

	rpcnode.add_delegate("hello", igris::make_delegate(hello));
	rpcnode.add_delegate("add", igris::make_delegate(func_add));
	rpcnode.add_delegate("sub", igris::make_delegate(func_sub));
	rpcnode.add_delegate("mul", igris::make_delegate(func_mul));
	rpcnode.add_delegate("div", igris::make_delegate(func_div));

	crow::start_spin();
	crow::join_spin();
}