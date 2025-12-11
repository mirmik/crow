#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include <crow/tower.h>
#ifdef __WIN32__
#include <winsock2.h>
WSADATA wsaData;
#endif

int main(int argc, char** argv)
{
#ifdef __WIN32__
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
#endif

	// Reset state before running tests
	crow::reset_for_test();

	// Verify reset worked
	if (crow::allocated_count() != 0) {
		printf("WARNING: allocated_count=%d after reset!\n", crow::allocated_count());
	}

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();

	if (context.shouldExit())
		return res;

	return res;
}
