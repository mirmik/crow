#include <doctest/doctest.h>
#include <crow/constexpr_hexer.h>
#include <array>

template< size_t N >
constexpr std::array<char, N> constexpr_char_array( char const (&s)[N] )
{
	std::array<char, N> ret = {};
	for (size_t i = 0; i < N; ++i)
		ret[i] = s[i];
	return ret;
}

TEST_CASE("constexpr_hexer")
{
	constexpr auto addr = constexpr_char_array(".12.127.0.0.1:10009");
	constexpr auto crowaddr = crow::constexpr_hexer(addr);

	CHECK_EQ(crowaddr.first[0], 12);
	CHECK_EQ(crowaddr.first[1], 127);
	CHECK_EQ(crowaddr.first[2], 0);
	CHECK_EQ(crowaddr.first[3], 0);
	CHECK_EQ(crowaddr.first[4], 1);
	CHECK_EQ(crowaddr.first[5], 39);
	CHECK_EQ(crowaddr.first[6], 25);
}