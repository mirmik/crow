/** @file */

#ifndef CROW_TOWER_THREAD_EXECUTOR_H
#define CROW_TOWER_THREAD_EXECUTOR_H

#include <crow/tower_cls.h>
#include <atomic>
#include <thread>

namespace crow
{
    /**
     * @brief Manages a dedicated thread for running a Tower's event loop.
     *
     * This class provides thread management for Tower instances in environments
     * that support std::thread. For embedded systems without thread support,
     * use Tower::onestep() directly in your main loop.
     *
     * Usage:
     *   crow::Tower tower;
     *   crow::TowerThreadExecutor executor(tower);
     *   executor.start();
     *   // ... do other work ...
     *   executor.stop();
     */
    class TowerThreadExecutor
    {
        Tower &_tower;
        std::thread _thread;
        std::atomic<bool> _cancel_token{false};
        std::atomic<bool> _running{false};

    public:
        explicit TowerThreadExecutor(Tower &tower) : _tower(tower) {}

        ~TowerThreadExecutor()
        {
            stop(true);
        }

        // Non-copyable, non-movable
        TowerThreadExecutor(const TowerThreadExecutor &) = delete;
        TowerThreadExecutor &operator=(const TowerThreadExecutor &) = delete;
        TowerThreadExecutor(TowerThreadExecutor &&) = delete;
        TowerThreadExecutor &operator=(TowerThreadExecutor &&) = delete;

        /**
         * @brief Start the executor thread.
         * @return 0 on success, -1 if already running.
         */
        int start();

        /**
         * @brief Stop the executor thread.
         * @param wait If true, wait for thread to finish.
         * @return 0 on success, -1 if not running.
         */
        int stop(bool wait = true);

        /**
         * @brief Wait for the executor thread to finish.
         */
        void join();

        /**
         * @brief Check if the executor is currently running.
         */
        bool running() const { return _running.load(); }

        /**
         * @brief Get the associated tower.
         */
        Tower &tower() { return _tower; }

    private:
        void thread_func();
    };

} // namespace crow

#endif
