#include <chrono>
#include <crow/asyncio.h>
#include <crow/tower.h>
#include <crow/tower_cls.h>
#include <crow/warn.h>
#include <igris/osutil/fd.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

static bool cancel_token = false;
static std::thread _thread;
bool _spin_runned = false;
bool _spin_runned_unbounded = false;

namespace crow
{

// Tower-specific spin function
void spin_with_select(Tower &tower)
{
    _spin_runned = true;

    while (1)
    {
        if (cancel_token)
        {
            break;
        }

        do
        {
            tower.onestep();

            if (cancel_token)
                break;
        } while (tower.has_untravelled_now());

        int64_t timeout = tower.get_minimal_timeout();
        asyncio.step(timeout);
    };

    _spin_runned = false;
}

// Compatibility: uses default_tower()
void spin_with_select()
{
    spin_with_select(default_tower());
}

void spin(Tower &tower)
{
    spin_with_select(tower);
}

void spin()
{
    spin_with_select();
}

int start_spin_with_select()
{
    if (_spin_runned)
    {
        throw std::runtime_error("spin thread double start");
    }

    cancel_token = false;
    _spin_runned_unbounded = true;
    _spin_runned = true;
    _thread = std::thread(static_cast<void(*)()>(spin_with_select));

    return 0;
}

int start_spin()
{
    return start_spin_with_select();
}

int start_spin_without_select()
{
    if (_spin_runned)
    {
        return -1;
    }

    _spin_runned_unbounded = true;
    _thread = std::thread(
        []()
        {
            _spin_runned = true;

            while (1)
            {
                if (cancel_token)
                    break;

                default_tower().onestep();

                if (cancel_token)
                    break;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            };

            _spin_runned = false;
        });

    return 0;
}

int stop_spin(bool wait)
{
    if (!_spin_runned)
    {
        throw std::runtime_error("thread is not started");
    }

    cancel_token = true;
    asyncio.cancel();

    if (wait)
        try
        {
            _thread.join();
        }
        catch (...)
        {
        }
    _spin_runned_unbounded = false;
    std::this_thread::sleep_for(100ms);
    return 0;
}

void spin_join()
{
    _thread.join();
}

void join_spin()
{
    _thread.join();
}

void set_spin_cancel_token()
{
    cancel_token = true;
}

//#if defined(CROW_REALTIME_THREADS)
#include <igris/osutil/realtime.h>
void spin_with_select_realtime(int abort_on_fault)
{
    int ret;
    if ((ret = this_thread_set_realtime_priority()))
    {
        crow::warn("Error on set_realtime_priority");

        if (abort_on_fault)
            abort();
    }

    spin_with_select(default_tower());
}

int start_spin_with_select_realtime(int abort_on_fault)
{
    if (_spin_runned)
    {
        throw std::runtime_error("spin thread double start");
    }

    cancel_token = false;
    _spin_runned_unbounded = true;
    _spin_runned = true;
    _thread = std::thread(spin_with_select_realtime, abort_on_fault);

    return 0;
}

int start_spin_realtime(int abort_on_fault)
{
    return start_spin_with_select_realtime(abort_on_fault);
}
//#endif

} // namespace crow