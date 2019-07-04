/// @file

#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include <igris/dprint.h>

#ifdef __cplusplus
#include <string>
#include <vector>
#endif

#define CROW_HEXER_UNDEFINED_SYMBOL -1
#define CROW_HEXER_MORE3_DOT -4
#define CROW_HEXER_ODD_GRID -5

__BEGIN_DECLS

/// Утилита для удобной записи хекс адреса.
/// Пример: .12.127.0.0.1#2714  -> 0C7F0000012714
/// Пример: .12.127.0.0.1:10004 -> 0C7F0000012714
/// Пример: .12.127:.1:10004 -> 0C7F0000012714
/// Пример: .12#7F000001:10004 -> 0C7F0000012714
/// Пример: #0C#7F000001#2714 -> 0C7F0000012714
/// Пример: #0C7F0000012714 -> 0C7F0000012714
int hexer(uint8_t *dst, size_t maxsz, const char *src, size_t srcsz);
int hexer_s(uint8_t *dst, size_t maxsz, const char *src);

__END_DECLS


#ifdef __cplusplus
[[deprecated]]
static inline
std::vector<uint8_t> compile_address(const std::string& in)
{
	std::vector<uint8_t> out;
	out.resize(in.size());

	int len = hexer_s((uint8_t*)out.data(), in.size(), in.data());
	out.resize(len);

	return out;
}

namespace crow
{
	static inline
	std::vector<uint8_t> address(const std::string& in)
	{
		std::vector<uint8_t> out;
		out.resize(in.size());

		int len = hexer_s((uint8_t*)out.data(), in.size(), in.data());
		
		if (len < 0)
			return {};

		out.resize(len);
		return out;
	}

	static inline
	std::vector<uint8_t> address_warned(const std::string& in)
	{
		std::vector<uint8_t> out;
		out.resize(in.size());

		int len = hexer_s((uint8_t*)out.data(), in.size(), in.data());
		
		if (len == CROW_HEXER_MORE3_DOT)
			dprln("crow::hexer: more then three symbols after dot.");

		if (len == CROW_HEXER_ODD_GRID)
			dprln("crow::hexer: odd symbols after #");

		if (len == CROW_HEXER_UNDEFINED_SYMBOL)
			dprln("crow::hexer: undefined symbol");

		if (len < 0)
			return {};

		out.resize(len);
		return out;
	}
}

#endif