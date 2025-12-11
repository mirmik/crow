#include <crow/tower_thread_executor.h>
#include <crow/asyncio.h>

namespace crow
{

int TowerThreadExecutor::start()
{
    if (_running.load())
    {
        return -1;
    }

    _cancel_token.store(false);
    _running.store(true);
    _thread = std::thread(&TowerThreadExecutor::thread_func, this);

    return 0;
}

int TowerThreadExecutor::stop(bool wait)
{
    if (!_running.load())
    {
        return -1;
    }

    _cancel_token.store(true);
    asyncio.cancel();

    if (wait)
    {
        join();
    }

    return 0;
}

void TowerThreadExecutor::join()
{
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void TowerThreadExecutor::thread_func()
{
    while (!_cancel_token.load())
    {
        do
        {
            _tower.onestep();

            if (_cancel_token.load())
                break;
        } while (_tower.has_untravelled_now());

        if (_cancel_token.load())
            break;

        int64_t timeout = _tower.get_minimal_timeout();
        asyncio.step(timeout);
    }

    _running.store(false);
}

} // namespace crow
