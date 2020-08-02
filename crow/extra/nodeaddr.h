#ifndef CROW_NODEADDR_H
#define CROW_NODEADDR_H

#include <crow/types.h>
#include <vector>

#include <crow/address.h>

namespace crow
{
	class nodeaddr
	{
	public:
		std::vector<uint8_t> naddr;
		nid_t nid;
	
	public:
		crow::hostaddr hostaddr() const
		{
			return { naddr.data(), naddr.size() };
		}

		bool operator< (const nodeaddr& oth) const 
		{
			if (naddr < oth.naddr) return true;
			if (nid < oth.nid) return true;
			return false;
		}
	};
}

#endif