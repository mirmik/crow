#ifndef CROW_WARN_H
#define CROW_WARN_H

#error "TO REMOVE"

#include <nos/print.h>

namespace crow
{
	template <class ... Args>
	void warning(const Args& ... args)
	{
		nos::println(args ...);
	}
}

#endif