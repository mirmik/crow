#include <crow/crow.h>

#include <thread>
#include <chrono>

#include <nos/fprint.h>
#include <getopt.h>

#include <crow/proto/rpc.h>

using namespace std::chrono_literals;

std::string funcname;
std::string strargs;
crow::hostaddr address;

void func()
{
	crow::rpc_requestor requestor(address);

	std::string result;
	int status = requestor.request_text_format(funcname.c_str(), result, strargs);
	
	switch (status)
	{
		case 0: nos::println(result);
			break;
		case CROW_ERRNO_UNREGISTRED_RID:
			nos::println("ConnectionError: Unregistred remote node id.");
			break;
		case CROW_RPC_ERROR_UNDELIVERED:
			nos::println("ConnectionError: Undelivered.");
			break;
		case CROW_RPC_ERROR_FUNCTION_NOT_FOUNDED:
			nos::println("RemoteError: Function not founded.");
			break;
		case CROW_RPC_ERROR_UNRESOLVED_FORMAT:
			nos::println("RemoteError: Wrong data format.");
			break;
		default: BUG();
	}

	crow::stop_spin();
}

void print_help()
{
	printf(
	    "Usage: crowrequest [OPTION] ADDRESS FUNC ARGS(TRENT)\n"
	    "example: crowrequest .12.127.0.0.1:10009 hello [42,38,35.12]\n"
	    "example: crowrequest .12.127.0.0.1:10009 hello []\n"
	    "\n"
	    "Common option list:\n"
	    "  -h, --help            print this page\n"
	    "\n"
	    "Info option list:\n"
	    "      --debug\n"
	    "\n"
	    "Crow address reference:\n"
	    "      man crow-protocol\n"
	    "Default RPC node: %d\n", 
	    CROW_RPC_NODE_NO
	);
}


int main(int argc, char** argv)
{
	const struct option long_options[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"debug", no_argument, NULL, 'd'},
		{NULL, 0, NULL, 0}
	};

	int long_index = 0;
	int opt = 0;

	//std::vector<const char *> fargs;

	while ((opt = getopt_long(argc, argv, "", long_options,
	                          &long_index)) != -1)
	{
		switch (opt)
		{
			case 'h':
				print_help();
				exit(0);

			case 'd':
				crow::diagnostic_setup(true, false);
				continue;

			case '?':
				dprln("HERE");
				continue;

			case 0:
				dprln("HERE");
				continue;

			default:
				BUG();
				break;
		}
	};

	if (optind + 2 != argc && optind + 3 != argc)
	{
		print_help();
		exit(-1);
	}

	address = crow::address(argv[optind]);
	funcname = argv[optind + 1];

	if (optind + 3 == argc)
		strargs = argv[optind + 2];
	else
		strargs = "[]";

	crow::create_udpgate(12, 0);

	std::thread thr(func);
	crow::start_spin();

	thr.join();
}
