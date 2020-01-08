#include <sys/timerfd.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>

#include <iostream>
#include <vector>

void SetTime(struct itimerspec& tsp,const time_t start, const time_t interval)
{
    bzero(&tsp, sizeof(tsp));
    tsp.it_value.tv_sec = start; //首次超时
    tsp.it_value.tv_nsec = 0;

    tsp.it_interval.tv_sec = interval; //周期性超时
    tsp.it_interval.tv_nsec = 0;
}

int CreateTimer(struct itimerspec& t)
{
    int tmfd;
    tmfd = timerfd_create(CLOCK_REALTIME, 0);
    assert(tmfd != -1 && "fail to create timer fd");
    int ret = timerfd_settime(tmfd, 0, &t, NULL);
    assert(ret != -1 && "fail to set time");
    return tmfd;
}

void EventLoop()
{
    struct itimerspec new_value;
    SetTime(new_value, 1, 1);
    int tmfd = CreateTimer(new_value);

    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    //用于存放返回的事件
    std::vector<epoll_event> ready_events;
    ready_events.reserve(1024);

    epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = tmfd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tmfd, &event);

    while(true)
    {
        int ret = epoll_wait(epoll_fd, &ready_events.front(), ready_events.size(), -1);
        std::cout << "awake" << std::endl;
        int64_t data;
        read(tmfd, &data, sizeof(data));
    }
}

int main()
{
    EventLoop();
    return 0;
}
