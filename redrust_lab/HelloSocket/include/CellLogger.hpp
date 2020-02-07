#ifndef __CELLLOGGER__H__
#define __CELLLOGGER__H__

#include <chrono>

#include "Cell.hpp"
#include "CellTask.hpp"

class CellLogger
{
private:
    FILE* _logFile = nullptr;
    CellTaskServer _taskServer;

    CellLogger(/* args */)
    {
        _taskServer.Start();
    }

    ~CellLogger()
   { 
        if(_logFile)
        {
            Info("CellLogger fclose()\n");
            fclose(_logFile);
            _logFile = nullptr;
        }
       _taskServer.Close();
    }

public:
    static CellLogger& Instance()
    {
        static CellLogger cellLogger;
        return cellLogger;
    }

    void setLogPath(const char* logPath,const char* mode)
    {
        if(_logFile)
        {
            Info("CellLogger::setLogPath _logFile != nullptr\n");
            fclose(_logFile);
            _logFile = nullptr;
        }
        _logFile = fopen(logPath,mode);
        if(_logFile)
        {
            Info("CellLogger::setLogPath success,<%s,%s>\n",logPath,mode);
        }else
        {
            Info("CellLogger::setLogPath failed,<%s,%s>\n",logPath,mode);
        }
    }

    void Info(const char* pStr)
    {
        CellLogger& pLog = Instance();
        if(pLog._logFile)
        {
            auto t = std::chrono::system_clock::now();
            auto tNow =  std::chrono::system_clock::to_time_t(t);
            //fprintf(pLog._logFile,"%s",ctime(&tNow));
            std::tm* now = std::gmtime(&tNow);
            fprintf(pLog._logFile,"[%d-%d-%d %d:%d:%d]",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday, now->tm_hour,now->tm_min,now->tm_sec);
            fprintf(pLog._logFile,"%s",pStr);
            fflush(pLog._logFile);
        }
        std::cout << pStr << std::endl;
    }

    template<typename ...Args>
    void Info(const char* pFormat,Args ... args)
    {
        CellLogger& pLog = Instance();
        pLog._taskServer.addTask([&](){
                if(pLog._logFile)
                {
                    auto t = std::chrono::system_clock::now();
                    auto tNow =  std::chrono::system_clock::to_time_t(t);
                    //fprintf(pLog._logFile,"%s",ctime(&tNow));
                    std::tm* now = std::gmtime(&tNow);
                    fprintf(pLog._logFile,"[%d-%d-%d %d:%d:%d]",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday, now->tm_hour,now->tm_min,now->tm_sec);
                    fprintf(pLog._logFile,"%s",pFormat);
                    fprintf(pLog._logFile,pFormat,args...);
                    fflush(pLog._logFile);
                }  
            }
        );
        printf(pFormat,args...);
    }

    void debug(const char* pStr)
    {

    }

    void warning(const char* ptr)
    {

    }

    void error(const char* pStr)
    {

    }
};

#endif  //!__CELLLOGGER__H__