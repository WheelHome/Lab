#ifndef MTHREAD_THREAD_HPP_
#define MTHREAD_THREAD_HPP_

#include <thread>
namespace mthread
{
    class Thread
    {
    public:
        Thread(const Thread&) = delete;

        Thread& operator=(const Thread&) = delete;

        Thread(Thread&& t)
        {
            m_thread = std::move(t.m_thread);
        }

        Thread() = default;

        template<typename F, typename... Args>
        void Run(F&& f, Args&&... args)
        {
            auto func = std::bind(f, args...);
            m_thread = std::thread(std::move(func));
        }

        ~Thread()
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }

    private:
        std::thread m_thread;
    };
};
#endif
