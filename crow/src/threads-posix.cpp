#include <crow/tower.h>
#include <crow/proto/pubsub.h>

#include <thread>
#include <chrono>

#include <crow/select.h>

#include <unistd.h>
#include <igris/osutil/fd.h>
#include <nos/print.h>

#include <signal.h>

static bool cancel_token = false;
static std::thread _thread;

int crow::unselect_pipe[2];

struct sigaction new_action, sigkill_old_action, sigint_old_action;

void signal_sigint_handler(int sig) 
{
	cancel_token = true;
	write(crow::unselect_pipe[1], "A", 1);
	_thread.join();
	sigint_old_action.sa_handler(sig);
}

void signal_sigkill_handler(int sig) 
{
	cancel_token = true;
	write(crow::unselect_pipe[1], "A", 1);
	_thread.join();
	sigkill_old_action.sa_handler(sig);
}


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

void crow::spin_with_select()
{
	crow::unselect_init();
	crow::select_collect_fds();

	sigaction (SIGKILL, NULL, &sigkill_old_action);
	signal(SIGKILL, signal_sigkill_handler);

	sigaction (SIGINT, NULL, &sigint_old_action);
	signal(SIGINT, signal_sigint_handler);

	while (1)
	{
		char unselect_read_buffer[512];

		if (cancel_token) 
		{
			break;
		}

		do
		{
			crow::onestep();
		}
		while (crow::has_untravelled_now());
		
		crow::select();
		read(unselect_pipe[0], unselect_read_buffer, 512);
	};
}

void crow::start_spin_with_select()
{
	_thread = std::thread(spin_with_select);
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
	crow::unselect();
	//_thread.join();
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

void crow::spin_join() 
{
	_thread.join();
}

void crow::join_spin() 
{
	_thread.join(); 
}
