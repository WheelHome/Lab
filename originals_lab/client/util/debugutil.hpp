#ifndef DEBUGUTIL_HPP_
#define DEBUGUTIL_HPP_

#include <iostream>
#include <unistd.h>
#include <mutex>


namespace Debug
{

#define __DEBUG_INFO __FILE__,__FUNCTION__,__LINE__

class DebugUtil
{
public:
    static DebugUtil* GetInstance()
    {
        static DebugUtil tool;
        return &tool;
    }

    template<typename... Args>
    void Trace(const char* file, const char* function, int line, Args&&... args)
    {
        std::unique_lock<std::mutex> lk(m_mut, std::defer_lock);
        if (m_is_lock)
        {
            lk.lock();
        }
        PrintDebugInfo(std::cout, file, function, line);
        Print(std::cout, args...);
    }

    template<typename... Args>
    void Warn(const char* file, const char* function, int line, Args&&... args)
    {
        std::unique_lock<std::mutex> lk(m_mut, std::defer_lock);
        if (m_is_lock)
        {
            lk.lock();
        }
        PrintDebugInfo(std::cerr, file, function, line);
        Print(std::cerr, args...);
    }

    void SetLock(bool lock)
    {
        m_is_lock = lock;
    }

    void SetWidth(const std::streamsize& width)
    {
        if (width < 0)
            return;
        m_size = width;
    }

    void Stop(int32_t i)
    {
        std::cout << "============================DEBUG STOP===========================" << std::endl;
        sleep(i);
    }

    void LogFile(bool s)
    {
        m_log_file = s;
    }

    void LogFunction(bool s)
    {
        m_log_function = s;
    }

    void LogLine(bool s)
    {
        m_log_file = s;
    }

private:
    DebugUtil()
        : m_is_lock(false),
          m_log_file(true),
          m_log_function(true),
          m_log_line(true),
          m_size(10)
    {} ;

    virtual ~DebugUtil() = default;

    void PrintDebugInfo(std::ostream& out, const char* file, const char* function, int line)
    {
        out.setf(std::ios::left);
        out.width(m_size);

        if (m_log_file)
            out << " [ FILE ] " << file;

        if (m_log_function)
            out << " [Function] " << function;

        if (m_log_line)
            out << " [Line] " << line;

        out << " :: ";
    }

    template<typename T>
    void Print(std::ostream& out, T&& data)
    {
        out << data << std::endl;
    }

    template<typename T, typename... Args>
    void Print(std::ostream& out, T&& data, Args&&... args)
    {
        out << data;
        Print(out, args...);
    }

    std::mutex m_mut;
    bool m_is_lock;
    bool m_log_file;
    bool m_log_function;
    bool m_log_line;
    std::streamsize m_size;
};

#ifdef DEBUG

#define TRACE(...)     do{Debug::DebugUtil::GetInstance()->Trace(__DEBUG_INFO, __VA_ARGS__);} while(0)
#define WARN(...)      do{Debug::DebugUtil::GetInstance()->Warn(__DEBUG_INFO, __VA_ARGS__);} while(0)
#define STOP(i)        do{Debug::DebugUtil::GetInstance()->Stop(i);} while(0)
#define SetLogLock(i)  do{Debug::DebugUtil::GetInstance()->SetLock(i);} while(0)
#define SetLogWidth(i) do{Debug::DebugUtil::GetInstance()->SetWidth(i);} while(0)
#define LogFile(i) do{Debug::DebugUtil::GetInstance()->LogFile(i);} while(0)
#define DisableFunction(i) do{Debug::DebugUtil::GetInstance()->LogFunction(i);} while(0)
#define DisableLine(i) do{Debug::DebugUtil::GetInstance()->LogLine(i);} while(0)

#else

#define TRACE(...)    do{} while(0)
#define WARN(...)     do{} while(0)
#define STOP(i)       do{} while(0)
#define SetLogLock(i) do{} while(0)
#define SetLogWidth(i) do{} while(0)

#endif
}
#endif
