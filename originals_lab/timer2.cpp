#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <queue>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>


struct Task
{
    Task(const time_t& time, std::function<void()> f)
        : m_interval(time),
          m_handler(f) {}
    time_t m_interval;
    std::function<void()> m_handler;
};

using task_ptr = std::shared_ptr<Task>;

class TaskMgr
{
public:
    TaskMgr()
        : m_tasks([](const task_ptr& x, const task_ptr& y)
    {
        return x->m_interval > y->m_interval;
    }),
    m_offset(0),
    m_id(0)
    {
    }

    ~TaskMgr() = default;

    int AddTask(const time_t& interval, std::function<void()> handler)
    {
        m_id++;
        auto ptr = std::make_shared<Task>(interval, handler);
        m_table[m_id] = ptr;
        m_tasks.emplace(ptr);
        return m_id;
    }

    void print_queue()
    {
        while(!m_tasks.empty())
        {
            std::cout << m_tasks.top()->m_interval << std::endl;
            m_tasks.pop();
        }
    }

private:
    std::priority_queue<task_ptr, std::vector<task_ptr>, std::function<bool(const task_ptr&, const task_ptr&)>> m_tasks;
    time_t m_offset;
    std::unordered_map<int, task_ptr> m_table;
    uint64_t m_id;
};

int main()
{
    TaskMgr mgr;
    mgr.AddTask(10, [] {});
    mgr.AddTask(20, [] {});
    mgr.AddTask(5, [] {});
    mgr.AddTask(30, [] {});
    mgr.print_queue();
    return 0;
}
