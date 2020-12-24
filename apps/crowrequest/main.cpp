#include <crow/crow.h>

#include <thread>
#include <chrono>

#include <nos/fprint.h>

using namespace std::chrono_literals;

std::string funcname;
std::string strargs;
std::vector<uint8_t> address;

void func() 
{
	crow::rpc_requestor requestor(address);
	
	std::string result;
	int status = requestor.request_text_format(funcname.c_str(), result, strargs);

	if (status == 0) 
	{
		nos::println(result);
	}

	else 
	{
		nos::fprintln("Error, status:{}", status);
	}

	crow::stop_spin();
}

int main(int argc, char** argv) 
{
	if (argc != 4) 
	{
		nos::println("usage: crowrequest ADDRESS FUNCTION TRENT");
		exit(0);
	} 

	address = crow::address(argv[1]);
	funcname = argv[2];
	strargs = argv[3];
	
	crow::create_udpgate(12, 0);
	
	std::thread thr(func);

	crow::start_spin();
	crow::join_spin();

	thr.join();
}
