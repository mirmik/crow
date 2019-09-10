#ifndef CROW_ADDONS_NOSPUBLISHER_H
#define CROW_ADDONS_NOSPUBLISHER_H

#include <nos/io/ostream.h>

namespace crow
{
	class nospublisher : public nos::ostream
	{
		uint8_t * addr;
		uint8_t alen;
		const char* theme;

	public:
		void init(
		    uint8_t * addr,
		    uint8_t alen,
		    const char* theme) 
		{ 
			this->addr = addr; 
			this->alen = alen;
			this->theme = theme;
		}

	public:
		ssize_t write(const void* ptr, size_t sz) override
		{
			crow::publish(addr, alen, theme, ptr, sz, 0, 200);
		}
	};
}

#endif