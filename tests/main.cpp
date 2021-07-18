#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include <crow/gates/udpgate.h>
#include <crow/gates/loopgate.h>
#include <crow/tower.h>

int main(int argc, char** argv)
{
	crow::retransling_allowed = true;
	crow::create_loopgate(99);
	crow::create_udpgate(12, 10099);
	//crow::start_spin();
	//crow::diagnostic_setup(true, true);

	doctest::Context context;

	int res = context.run(); // run

	if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
		return res;          // propagate the result of the tests

	//crow::stop_spin();

	int client_stuff_return_code = 0;
	// your program - if the testing framework is integrated in your production code

	return res + client_stuff_return_code; // the result from doctest is propagated here as well
}