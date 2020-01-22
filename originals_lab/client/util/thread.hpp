#ifndef THREAD_HPP_
#define THREAD_HPP_
#include <thread>
#include <sys/eventfd.h>
#include <assert.h>
#include <unistd.h>


class ThreadWapper
{
public:
    ThreadWapper(const ThreadWapper&) = delete;
    ThreadWapper& operator=(const ThreadWapper&) = delete;

    ThreadWapper() = default;

    void Set(std::thread&& thread)
        {
            m_thread = std::move(thread);
        }

    ~ThreadWapper()
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }

private:
    std::thread m_thread;
};

struct ThreadInfo
{
    ThreadInfo()
        {
            m_event_fd = eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE);
            assert(m_event_fd != -1);
        }

    void Notifity()
        {
            int64_t data = 1;
            write(m_event_fd, &data, sizeof(data));
        }

    void Reset()
        {
            int64_t data;
            read(m_event_fd, &data, sizeof(data));
        }

    ThreadWapper m_thread;
    int m_event_fd;
};

#endif
