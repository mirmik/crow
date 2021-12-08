#include <crow/tower.h>
#include <crow/warn.h>

#include <chrono>
#include <thread>

#include <crow/asyncio.h>

#include <igris/osutil/fd.h>
#include <igris/osutil/realtime.h>
#include <unistd.h>

#include <signal.h>

static bool cancel_token = false;
static std::thread _thread;
bool _spin_runned = false;
bool _spin_runned_unbounded = false;

struct sigaction new_action, sigkill_old_action, sigint_old_action;

void signal_sigint_handler(int sig)
{
    if (_spin_runned)
    {
        crow::asyncio.cancel();
        cancel_token = true;
        if (_spin_runned_unbounded)
            _thread.join();
    }
    if (sigint_old_action.sa_handler)
        sigint_old_action.sa_handler(sig);
    else
        exit(0);
}

void crow::spin_with_select()
{
    _spin_runned = true;

    sigaction(SIGINT, NULL, &sigint_old_action);
    signal(SIGINT, signal_sigint_handler);

    while (1)
    {
        if (cancel_token)
        {
            break;
        }

        do
        {
            crow::onestep();

            if (cancel_token)
                break;
        }
        while (crow::has_untravelled_now());

        int64_t timeout = crow::get_minimal_timeout();
        asyncio.step(timeout);
    };

    _spin_runned = false;
}

void crow::spin_with_select_realtime(int abort_on_fault)
{
    int ret;
    if ((ret = this_thread_set_realtime_priority()))
    {
        crow::warn("Error on set_realtime_priority");

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
    _thread = std::thread([]()
    {
        _spin_runned = true;

        while (1)
        {
            if (cancel_token)
                break;

            crow::onestep();

            if (cancel_token)
                break;
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
    asyncio.cancel();

    if (wait)
        _thread.join();
    _spin_runned_unbounded = false;
    return 0;
}

void crow::spin_join()
{
    _thread.join();
}

void crow::join_spin()
{
    _thread.join();
}

void crow::set_spin_cancel_token()
{
    cancel_token = true;
}