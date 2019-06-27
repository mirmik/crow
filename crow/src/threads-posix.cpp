#include <crow/tower.h>
#include <crow/pubsub.h>

#include <thread>
#include <chrono>

#define NODTRACE 1
#include <igris/dtrace.h>

void crow::start_thread()
{
	DTRACE();
	std::thread thr([]()
	{
		while (1)
		{
			crow::onestep();
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		};
	});
	thr.detach();
}

void crow::pubsub_protocol_cls::start_resubscribe_thread(int millis)
{
	DTRACE();
	std::thread thr([ = ]()
	{
		while (1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(millis));
			crow::pubsub_protocol.resubscribe_all();
		}
	});
	thr.detach();
}