#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <chrono>

class Timer
{
public:
    Timer() = default;

    ~Timer() = default;

    void Start()
    {
        m_ms = 0;
        m_start = std::chrono::system_clock::now();
    }

    void Stop()
    {
        m_end = std::chrono::system_clock::now();
        m_ms += std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count();
    }

    void Accumulate()
    {
        m_end = std::chrono::system_clock::now();
        m_ms += std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count();
        m_start = m_end;
    }

    unsigned int MS()
    {
        return m_ms;
    }

    unsigned int S()
    {
        return MS() / 1000;
    }

private:
    using sys_time_point = std::chrono::time_point<std::chrono::system_clock>;

    sys_time_point m_start;
    sys_time_point m_end;
    unsigned int m_ms;
};

#endif
