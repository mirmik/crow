#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include <crow/gates/udpgate.h>
#include <crow/gates/loopgate.h>
#include <crow/tower.h>

crow::udpgate udpgate;
crow::loopgate loopgate;

int main(int argc, char** argv)
{
	crow::retransling_allowed = true;
	loopgate.bind(99);
	udpgate.bind(12);
	udpgate.open(10099);


	doctest::Context context;

	int res = context.run(); // run

	if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
		return res;          // propagate the result of the tests

	//crow::stop_spin();

	int client_stuff_return_code = 0;
	// your program - if the testing framework is integrated in your production code

	return res + client_stuff_return_code; // the result from doctest is propagated here as well
}