#include <crow/hostaddr.h>

crow::hostaddr_view crow::hostaddr::view() 
{
	return hostaddr_view(_addr.data(), _addr.size());
}


crow::hostaddr::operator crow::hostaddr_view() 
{
	return view();
}

crow::hostaddr::hostaddr(const crow::hostaddr_view & h) 
{
	_addr = std::vector<uint8_t>(h.data(), h.data()+h.size());
}