#include <crow/crow.h>
#include <nos/print.h>

crow::udpgate ugate;

auto addr = crow::address(".12.127.0.0.1:10010");

void func() 
{
	crow::rpc_requestor requestor(addr);

	float out;
	int status = requestor.request<float, int, float>("hello", out, 23, 25.7);

	PRINT(status);
	PRINT(out);

	crow::stop_spin();
}

int main() 
{
	ugate.bind();
	ugate.open();

	crow::diagnostic_enable();

	std::thread thr(func);

	crow::start_spin();
	crow::join_spin();

	thr.join();
}