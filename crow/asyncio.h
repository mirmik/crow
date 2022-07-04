#ifndef CROW_ASYNCIO_H
#define CROW_ASYNCIO_H

#include <stdint.h>
#include <unordered_map>
#include <vector>

#include <sys/select.h>
#include <unistd.h>

#include <igris/event/delegate.h>
#include <igris/math.h>
#include <igris/osutil/fd.h>

#include <nos/print.h>

namespace crow
{
    enum class SelectType
    {
        LISTEN,
        READ
    };

    class asyncio_manager
    {
        struct record
        {
            int fd=-1;
            SelectType type={};
            igris::delegate<void, int> handler={};
        };

        bool cancel_token = false;
        int unsleep_pipe[2]={-1,-1};
        std::unordered_map<int, record> dict={};
        std::vector<int> fds={};

    public:
        asyncio_manager() { unsleep_subsystem_init(); }

        void unsleep()
        {
            char c = 42;
            int ret = ::write(unsleep_pipe[1], &c, 1);
            (void)ret;
        }

        void unsleep_handler(int fd)
        {
            char unselect_read_buffer[16];
            int unsret = read(fd, unselect_read_buffer, 16);
            (void)unsret;
        }

        void cancel()
        {
            cancel_token = true;
            unsleep();
        }

        void unsleep_subsystem_init()
        {
            int ret = ::pipe(unsleep_pipe);
            (void)ret;
            igris::osutil::nonblock(unsleep_pipe[0], true);
            add_iotask(
                unsleep_pipe[0], SelectType::READ,
                igris::make_delegate(&asyncio_manager::unsleep_handler, this));
        }

        void add_iotask(int fd, SelectType type,
                        igris::delegate<void, int> handler)
        {
            dict[fd] = {fd, type, handler};
            fds.push_back(fd);
            unsleep();
        }

        void step(int64_t timeout_ms)
        {
            if (cancel_token) 
                return;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            int nfds = 0;

            for (int i : fds)
            {
                FD_SET(i, &read_fds);
                nfds = __MAX__(i, nfds);
            }

            if (timeout_ms < 0)
            {
                ::select(nfds + 1, &read_fds, NULL, NULL, NULL);
            }
            else
            {
                struct timeval timeout_struct = {
                    (time_t)(timeout_ms / 1000),
                    (suseconds_t)((timeout_ms % 1000) * 1000)};
                ::select(nfds + 1, &read_fds, NULL, NULL, &timeout_struct);
            }

            for (int fd = 0; fd < nfds + 1; fd++)
                if (FD_ISSET(fd, &read_fds))
                {
                    dict[fd].handler(fd);
                }
        }
    };

    extern asyncio_manager asyncio;
}

#endif