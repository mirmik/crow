#ifndef CROW_PRINT_H
#define CROW_PRINT_H

#include <nos/print.h>

namespace crow
{
	void diagnostic(const char *notation, crow::packet *pack);

	template <class ... Args>
	void warning(const Args& ... args)
	{
		nos::println(args ...);
	}
}

#endif