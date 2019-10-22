#include "tcp.h"
#include <time.h>

int main(int argc, char *argv[])
{
    int listenfd,connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    time_t ticks;

    listenfd = socket(AF_INET,SOCK_STREAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8000);

    bind(listenfd,(sockaddr*)&servaddr,sizeof(servaddr));

    listen(listenfd,8);

    /*
    for (;;) {
        connfd = accept(listenfd,(sockaddr*)nullptr,nullptr);
        ticks = time(nullptr);
        snprintf(buff,sizeof(buff),"%.24s\r\n",ctime(&ticks));
        write(connfd,buff,strlen(buff));
        close(connfd);
    }*/
    for (;;)
    {
        if(!connfd)
            connfd = accept(listenfd,(sockaddr*)nullptr,nullptr);
        int n = read(connfd,buff,MAXLINE);
        buff[n] = 0;
        if(n)
            std::cout<<buff<<std::endl;

        std::string sendMessage;
        std::cin>>sendMessage;
        write(connfd,sendMessage.c_str(),sendMessage.size());
        if(sendMessage=="#")
            close(connfd);
    }
    return 0;
}
