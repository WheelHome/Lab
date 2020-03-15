#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include <thread>
#include <string>
#include <iostream>
#include  <string.h>

std::string g_pipename = "epoll_pipe";
int g_exit_fd = -1;

void Epoll()
{
    int listen_fd,socket_fd;
    int re = 0;
    int efd;
    int n = 0;
    struct sockaddr_in servaddr;
    struct epoll_event event;
    struct epoll_event exit_event;
    struct epoll_event* events;

    listen_fd = socket(AF_INET,SOCK_STREAM,0);

    if(listen_fd == -1)
    {
        std::cout << "Socket error!" << std::endl;
        return ;
    }
    int val = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEPORT,&val,sizeof(val)) < 0)
    {
        std::cout << "setsocket error!" << std::endl;
        return;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(12345);
    re = bind(listen_fd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if(re < 0)
    {
        std::cout << "Bind error!" << std::endl;
        close(listen_fd);
        return;
    }
    re = listen(listen_fd,1024);
    if(re < 0)
    {
        std::cout << "Listen error!" << std::endl;
        close(listen_fd);
        return;
    }
    efd = epoll_create1(0);
    if(efd == -1)
    {
        std::cout << "epoll create failed" << std::endl;
        close(listen_fd);
        return;
    }

    exit_event.data.fd = g_exit_fd;
    exit_event.events =  EPOLLIN | EPOLLET | EPOLLRDHUP;
    int ret = epoll_ctl(efd, EPOLL_CTL_ADD, g_exit_fd, &exit_event);
    assert(ret != -1 && "fail to register exit event");

    event.data.fd = listen_fd;
    event.events =  EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(epoll_ctl(efd,EPOLL_CTL_ADD,listen_fd,&event) == -1)
    {
        std::cout << "epoll_ctl add listen_fd failed!" << std::endl;
        return;
    }

    events = (epoll_event*)calloc(1024,sizeof(event));
    bool is_stop = false;
    while(!is_stop)
    {
        n = epoll_wait(efd,events,1024,-1);
        for(int i = 0 ; i < n ; i ++)
        {
            // 处理错误事件
            if((events[i].events & EPOLLERR) || !(events[i].events & EPOLLIN))
            {
                std::cout << events[i].data.fd << " epoll error!" << std::endl;
                shutdown(events[i].data.fd,SHUT_RDWR);
                continue;
            }

            if(listen_fd == events[i].data.fd)
            {
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);
                int fd  =accept(listen_fd,(struct sockaddr*)&ss,&slen);
                if(fd < 0)
                {
                    std::cout << "accept error! " << std::endl;
                    continue;
                }
                event.data.fd = fd;
                event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1)
                {
                    std::cout << "epoll_ctl client add listen_fd failed!" << std::endl;
                    close(fd);
                }
            }
            else if (g_exit_fd == events[i].data.fd)
            {
                is_stop = true;
                close(g_exit_fd);
                close(listen_fd);
            }
        }
    }
}

void HandleKillSignal(int sig)
{
    int fd = open(g_pipename.c_str(), O_WRONLY);
    char i = 1;
    write(fd, &i, sizeof(i));
}

void StartEpoll()
{
    std::thread epoll_thread(Epoll);

    // 创建管道
    int ret = mkfifo(g_pipename.c_str(), S_IFIFO | 0666);
    assert(ret != -1 && "can't create exit pipe");

    //接收父进程发来的退出信号
    int fd = open(g_pipename.c_str(), O_RDONLY);
    char singal = 0;
    read(fd, &singal, sizeof(singal));

    //向epoll线程发送退出信号
    int64_t exit = 1;
    write(g_exit_fd, &exit, sizeof(exit));

    unlink(g_pipename.c_str());
    epoll_thread.join();
}

//! ./a.out [anything], 将会发送退出信号
int main(int argc, char** argv)
{
    if (argc == 1) //不带任何参数启动
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            //处理信号
            signal(SIGKILL, HandleKillSignal);
            signal(SIGINT, HandleKillSignal);
            signal(SIGTERM, HandleKillSignal);

            g_exit_fd = eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE);
            //子进程启动Epoll
            //父进程直接退出
            StartEpoll();
        }
    }
    else
    {
        int fd = open(g_pipename.c_str(), O_WRONLY);
        if (fd == -1) //管道没有创建，打开失败
        {
            std::cout << "no epoll is running" << std::endl;
            return -1;
        }
        char i = 1;
        write(fd, &i, sizeof(i));
        std::cout<< "send the exit signal to sub-process" << std::endl;
    }
    return 0;
}