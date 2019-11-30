#ifndef __HCPW_LOG_H__
#define __HCPW_LOG_H__

#include <string>
#include <stdint.h>
#include <stringstream>
#include <fstream>

namespace hcpw{

    //日志事件
    class LogEvent{
        public:
            typedef std::shared_ptr<LogEvent> ptr;
            LogEvent();
        private:
            const char* m_file = nullptr;   //文件名
            int32_t m_line = 0;             //行号
            uint32_t m_elapse = 0;          //程序启动开始到现在的毫秒数
            uint32_t m_thread = 0;           //线程id
            uint32_t m_fiberId = 0;         //协程id
            uint64_t m_time;                //时间戳
            std::string m_content;
    };

    //日志级别
    class LogLevel{
        public:
            enum Level{
                DEBUG = 1,
                INFO = 2,
                WARN = 3,
                ERROR = 4,
                FATAL = 5
            };
    };

    //日志格式化生成器
    class LogFormatter{
        public:
            typedef std::shared_ptr<LogFormatter> ptr;
            LogFormatter(const std::string& pattern);
            std::string format(LogEvent::ptr event);
        private:
            class FormatItem{
                public:
                    typedef std::shared_ptr<FormatItem> ptr;
                    virtual ~FormatItem(){}
                    virtual void format(std::ostream& os,LogEvent::ptr event) = 0;
            };
            void init();
        private:
            std::string m_pattern;
            std::vector<FormatItem::ptr> m_items;

    };

    //日志输出器
    class LogAppender{
        public:
            typedef std::shared_ptr<LogAppender> ptr;
            virtual ~LogAppender(){}
            virtual void log(LogLevel::Level level,const LogEvent& event) = 0;
            void setFormatter(LogFormatter::ptr val){ m_formatter = val; }
            LogFormatter::ptr getFormatter()const{ return m_formatter; }
        protected:
            LogLevel::Level m_level;
            LogFormatter::ptr m_formatter;
    };

    //日志生成器
    class Logger{
        public:
            typedef std::shared_ptr<Logger> ptr;
            Logger(const std::string name = "root");
            void log(LogLevel::Level level,LogEvent::ptr event);

            void debug(LogEvent::ptr event);
            void info(LogEvent::ptr event);
            void warn(LogEvent::ptr event);
            void error(LogEvent::ptr event);
            void fatal(LogEvent::ptr event);

            void addAppender(LogAppender::ptr appender);
            void delAppender(LogAppender::ptr appender);
            LogLevel::Level getLevel()const { return m_level; }
            void SetLevel(LogLevel::Level val){ m_level = val; }
        private:
            std::string m_name;             //日志名称
            LogLevel::LogLevel m_level;     //日志级别
            std::list<LogAppender::ptr> m_appenders;               //Appender集合
    };

    //输出定义到控制台的Appender
    class StdoutLogAppender:public LogAppender{
        public:
            typedef std::shared_ptr<StdoutLogAppender> ptr;
            virtual void log(LogLevel level,LogEvent::prt event)override;
        private:
    };

    //定义输出到文件的Appender
    class FileLogAppender:public LogAppender{
        public:
            typedef std::shared_ptr<FileLogAppender> ptr;
            FileLogAppender(const std::string& filename);
            virtual void log(LogLevel::Level level,LogEvent::ptr event)override;

            //重新打开文件，成功返回true
            bool reopen();
        private:
            std::string m_filename;
            std::ofstream m_filestream;
    };
}
#endif
