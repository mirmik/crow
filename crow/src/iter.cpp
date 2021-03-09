#include "crow/iter.h"

std::vector<crow::gateway*> crow::gates() 
{
	std::vector<crow::gateway*> ret;
	for(auto& ref : crow::gateway_list) 
	{
		ret.push_back(&ref);
	}

	return ret;
}

std::vector<crow::node*> crow::nodes() 
{
	std::vector<crow::node*> ret;
	for(auto& ref : crow::nodes_list) 
	{
		ret.push_back(&ref);
	}

	return ret;
}
