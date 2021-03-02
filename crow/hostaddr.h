#ifndef CROW_HOSTADDR_H
#define CROW_HOSTADDR_H

#include <igris/buffer.h>
#include <vector>

namespace std 
{
	//template<class T> 
	//	class allocator;
	
	//template<typename _Tp, typename _Alloc> 
	//	class vector;
}

namespace crow
{
	class hostaddr
	{
	public:
		const void * addr;
		size_t       alen;

		hostaddr() = default;
		hostaddr(const igris::buffer & v) : addr(v.data()), alen(v.size()) {}
		hostaddr(const void * addr, size_t alen) : addr(addr), alen(alen) {}

		template <class Alloc>
		hostaddr(const std::vector<uint8_t, Alloc> & v) : addr(v.data()), alen(v.size()) {}

		const void * data() const { return addr; }
		size_t size() const { return alen; }

		bool operator == (igris::buffer buf) 
		{
			return alen == buf.size() && memcmp(addr, buf.data(), alen) == 0;
		}
	};
}

#endif
