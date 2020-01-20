#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <queue>
#include <functional>
#include <optional>

struct Task
{
    time_t m_time;
    std::function<void()> m_handler;
};

class TimerTaskMgr
{
public:
    TimerTaskMgr() = default;

    ~TimerTaskMgr() = default;

    void AddTask(const time_t& time, std::function<void()> func)
    {
        Task task;
        task.m_time = time;
        task.m_handler = func;
        m_timer_tasks.emplace(task);
    }

    std::optional<timeval> GetTask()
    {
        if (m_timer_tasks.empty())
        {
            return {};
        }
        bool timeout = true;
        timeval ret;
        bzero(&ret, sizeof(ret));
        time_t now = time(NULL);
        while(!m_timer_tasks.empty() && timeout)
        {
            auto& task = m_timer_tasks.front();
            time_t interval = task.m_time - now;
            if (interval <= 0)
            {
                m_timer_tasks.pop();
                continue;
            }
            timeout = false;
            ret.tv_sec = interval;
            m_timer_tasks.pop();
        }
        return ret;
    }
private:
    std::queue<Task> m_timer_tasks;
};

int main()
{
    TimerTaskMgr mgr;

    time_t t = time(NULL);

    for(int i = 2; i < 12; i++)
    {
        mgr.AddTask(t + i, []{});
    }
    sleep(2);

    std::optional<timeval> ret;
    do {
         ret = mgr.GetTask();
         if (ret)
         {
             std::cout << ret->tv_sec << std::endl;
         }
    } while(ret);
    return 0;
}
