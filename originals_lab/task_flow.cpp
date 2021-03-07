#include <map>
#include <string>
#include <queue>
#include <set>
#include <iostream>

std::map<std::string, std::string> cache;

struct Task
{
    virtual void Handle() = 0;

    template<typename T>
    static Task* GetTask()
    {
        static T task;
        return &task;
    }
};

struct TaskFlow
{
    template<typename T>
    void Register(T* t)
    {
        if (m_task_set.find(t) == m_task_set.end())
        {
            m_task_set.emplace(t);
            m_task.emplace(t);
        }
    }

    template<typename T, typename... Args>
    void Register(T* t, Args... args)
    {
        if (m_task_set.find(t) == m_task_set.end())
        {
            m_task_set.emplace(t);
            m_task.emplace(t);
        }
        Register(args...);
    }

    void Run()
    {
        while(!m_task.empty())
        {
            auto* task = m_task.front();
            m_task.pop();
            task->Handle();
	    m_task_set.erase(task);
        }
    }
    std::set<Task*> m_task_set;
    std::queue<Task*> m_task;
};

TaskFlow task_flow;

struct TaskA : Task
{
    void Handle()
    {
        cache["A"] = "A";
        std::cout << cache["A"] << std::endl;
    }

    std::string GetData()
    {
        return cache["A"];
    }
};

struct TaskB : Task
{
    void Handle()
    {
        auto data = TaskA().GetData();
        cache["B"] = data + "B";
        std::cout << cache["B"] << std::endl;
    }

    std::string GetData()
    {
        return cache["B"];
    }
};

struct TaskC : Task
{
   void Handle()
    {
        auto data1 = TaskA().GetData();
        auto data2 = TaskB().GetData();
        cache["C"] = data1 + data2 + "C";
        std::cout << cache["C"] << std::endl;
    }

    std::string GetData()
    {
        return cache["B"];
    }
};

int main()
{
    auto* a = Task::GetTask<TaskA>();
    auto* b = Task::GetTask<TaskB>();
    auto* c = Task::GetTask<TaskC>();
    task_flow.Register(a,b);
    task_flow.Register(a,b,c);
    task_flow.Register(b,c);
    task_flow.Run();
    return 0;
}
