#include <crow/pubsub/pubsub.h>
#include <crow/tower.h>

#include <chrono>
#include <thread>

#include <crow/select.h>

#include <igris/osutil/fd.h>
#include <igris/osutil/realtime.h>
#include <unistd.h>

#include <signal.h>

static bool cancel_token = false;
static std::thread _thread;
bool _spin_runned = false;
bool _spin_runned_unbounded = false;

int crow::unselect_pipe[2];

struct sigaction new_action, sigkill_old_action, sigint_old_action;

void signal_sigint_handler(int sig)
{
    if (_spin_runned)
    {
        cancel_token = true;
        write(crow::unselect_pipe[1], "A", 1);
        if (_spin_runned_unbounded)
            _thread.join();
    }
    if (sigint_old_action.sa_handler)
        sigint_old_action.sa_handler(sig);
    else
        exit(0);
}

/*void signal_sigkill_handler(int sig)
{
    if (_spin_runned)
    {
        cancel_token = true;
        write(crow::unselect_pipe[1], "A", 1);
        if (_spin_runned_unbounded)
            _thread.join();
    }
    if (sigkill_old_action.sa_handler)
        sigkill_old_action.sa_handler(sig);
    else
        exit(0);
}*/

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
    _spin_runned = true;

    crow::unselect_init();
    crow::select_collect_fds();

//    sigaction(SIGKILL, NULL, &sigkill_old_action);
//    signal(SIGKILL, signal_sigkill_handler);

    sigaction(SIGINT, NULL, &sigint_old_action);
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
        } while (crow::has_untravelled_now());

        crow::select();
        read(unselect_pipe[0], unselect_read_buffer, 512);
    };

    _spin_runned = false;
}

void crow::spin_with_select_realtime(int abort_on_fault)
{
    int ret;
    if ((ret = this_thread_set_realtime_priority()))
    {
        dprln("Error on set_realtime_priority", ret);

        if (abort_on_fault)
            abort();
    }

    crow::spin_with_select();
}

int crow::start_spin_with_select()
{
    if (_spin_runned)
    {
        return -1;
    }

    cancel_token = false;
    _spin_runned_unbounded = true;
    _thread = std::thread(spin_with_select);

    return 0;
}

int crow::start_spin_with_select_realtime(int abort_on_fault)
{
    if (_spin_runned)
    {
        return -1;
    }

    cancel_token = false;
    _spin_runned_unbounded = true;
    _thread = std::thread(spin_with_select_realtime, abort_on_fault);

    return 0;
}

int crow::start_spin() { return crow::start_spin_with_select(); }

int crow::start_spin_realtime(int abort_on_fault)
{
    return crow::start_spin_with_select_realtime(abort_on_fault);
}

int crow::start_spin_without_select()
{
    if (_spin_runned)
    {
        return -1;
    }

    _spin_runned_unbounded = true;
    _thread = std::thread([]() {
        _spin_runned = true;

        while (1)
        {
            if (cancel_token)
                break;

            crow::onestep();
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        };

        _spin_runned = false;
    });

    return 0;
}

int crow::stop_spin(bool wait)
{
    if (!_spin_runned)
    {
        return -1;
    }

    cancel_token = true;
    crow::unselect();

    if (wait)
        _thread.join();
    _spin_runned_unbounded = false;
    return 0;
}

void crow::pubsub_protocol_cls::start_resubscribe_thread(int millis)
{
    std::thread thr([=]() {
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(millis));
            crow::pubsub_protocol.resubscribe_all();
        }
    });
    thr.detach();
}

void crow::start_resubscribe_thread(int millis) 
{
	crow::pubsub_protocol_cls::start_resubscribe_thread(millis);
}

void crow::spin_join() { _thread.join(); }

void crow::join_spin() { _thread.join(); }
