#include <iostream>
#include <map>
#include <vector>

struct Task
{
    virtual void Handle() = 0;

    template <typename T>
    static Task* GetTask()
    {
        static T task;
        return &task;
    }
};

struct TaskNode
{
    std::vector<TaskNode> m_dep;
    Task* m_task;
};

struct TaskFlow
{
    int m_cur_level = 0;
    std::map<Task*, int64_t> m_level_table;

    void Register(TaskNode node)
    {
        m_cur_level = 0;
        m_level_table.clear();
        ScanDep({node});
    }

    void ScanDep(std::vector<TaskNode> node_vect)
    {
        std::vector<TaskNode> sub_node;
        for (auto& data : node_vect)
        {
            m_level_table[data.m_task] = m_cur_level;
            sub_node.insert(sub_node.end(), data.m_dep.begin(), data.m_dep.end());
        }
        m_cur_level++;
        if (!sub_node.empty())
        {
            ScanDep(sub_node);
        }
    }
};


struct TaskA : Task
{
    void Handle() override
    {
        std::cout << "A" << std::endl;
    }
};

struct TaskB : Task
{
    void Handle() override
    {
        std::cout << "B" << std::endl;
    }
};

struct TaskC : Task
{
    void Handle() override
    {
        std::cout << "C" << std::endl;
    }
};


struct TaskD : Task
{
    void Handle() override
    {
        std::cout << "D" << std::endl;
    }
};

struct TaskE : Task
{
    void Handle() override
    {
        std::cout << "F" << std::endl;
    }
};


std::map<Task*, std::string> task_name_table;
int main()
{
    TaskNode a;
    a.m_task = Task::GetTask<TaskA>();
    task_name_table[a.m_task] = "A";

    TaskNode b;
    b.m_task = Task::GetTask<TaskB>();
    task_name_table[b.m_task] = "B";

    TaskNode c;
    c.m_task = Task::GetTask<TaskC>();
    task_name_table[c.m_task] = "C";
    c.m_dep.emplace_back(a);
    c.m_dep.emplace_back(b);

    TaskNode d;
    d.m_task = Task::GetTask<TaskD>();
    task_name_table[d.m_task] = "D";
    d.m_dep.emplace_back(c);

    TaskNode e;
    e.m_task = Task::GetTask<TaskE>();
    task_name_table[e.m_task] = "E";
    e.m_dep.emplace_back(c);
    e.m_dep.emplace_back(d);

    TaskFlow taskflow;
    taskflow.Register(e);

    for (auto& data : taskflow.m_level_table)
    {
        std::cout << task_name_table[data.first] << ":" << data.second << std::endl;
    }
    return 0;
}
