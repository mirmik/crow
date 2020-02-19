#include <crow/tower.h>
#include <crow/proto/pubsub.h>

#include <thread>
#include <chrono>

static bool cancel_token = false;
static std::thread _thread;

void crow::start_thread()
{
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

void crow::start_spin()
{
	_thread = std::thread([]()
	{
		while (1)
		{
			if (cancel_token)
				return;

			crow::onestep();
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		};
	});
	//_thread.detach();
}

void crow::stop_spin() 
{
	cancel_token = true;
	_thread.join();
}

void crow::pubsub_protocol_cls::start_resubscribe_thread(int millis)
{
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