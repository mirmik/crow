#include <crow/tower.h>
#include <crow/proto/pubsub.h>

#include <thread>
#include <chrono>

#include <crow/select.h>

#include <unistd.h>
#include <igris/osutil/fd.h>
#include <nos/print.h>

static bool cancel_token = false;
static std::thread _thread;

int crow::unselect_pipe[2];

void crow::unselect()
{
	char c = 42;
	::write(unselect_pipe[1], &c, 1);
}

void crow::unselect_init() 
{
	crow::add_unselect_to_fds = true;
	::pipe(unselect_pipe);
	igris::osutil::nonblock(unselect_pipe[0], true);
	crow::unsleep_handler = unselect;	
}

void crow::start_spin_with_select()
{
	crow::unselect_init();

	_thread = std::thread([]()
	{
		crow::select_collect_fds();
		while (1)
		{
			char unselect_read_buffer[512];

			if (cancel_token)
				return;

			crow::select();
			read(unselect_pipe[0], unselect_read_buffer, 512);
			do
				crow::onestep();
			while (crow::has_untravelled_now());
		};
	});
}

void crow::start_spin() { crow::start_spin_with_select(); }

void crow::start_spin_without_select()
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