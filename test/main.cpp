#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include <crow/gates/udpgate.h>
#include <crow/tower.h>

int main(int argc, char** argv)
{
	crow::retransling_allowed = true;
	crow::create_udpgate(12, 10999);
	crow::start_spin();
	//crow::diagnostic_setup(true, true);

	doctest::Context context;

	// !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

	// defaults
	context.addFilter("test-case-exclude", "*math*"); // exclude test cases with "math" in their name
	context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
	context.setOption("order-by", "name");            // sort the test cases by their name

	context.applyCommandLine(argc, argv);

	// overrides
	context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

	int res = context.run(); // run

	if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
		return res;          // propagate the result of the tests

	crow::stop_spin();

	int client_stuff_return_code = 0;
	// your program - if the testing framework is integrated in your production code

	return res + client_stuff_return_code; // the result from doctest is propagated here as well
}