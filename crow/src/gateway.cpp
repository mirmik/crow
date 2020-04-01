#include <crow/gateway.h>
#include <crow/tower.h>

crow::gateway& crow::gateway::bind(int gateno) 
{
	crow::link_gate(this, gateno);
	return *this;
}