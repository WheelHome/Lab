#include "tcp.h"
#include <time.h>

void Read(int connfd)
{
    fflush(stdin);

    char temp[1024] = {0};
    int n = readn(connfd,temp,MAXLINE);
    temp[n] = '\0';
    std::cout<<temp<<std::endl;
}
void Send(int connfd)
{
    fflush(stdin);
    std::string buf;
    std::cin>>buf;
    int n = writen(connfd,buf.c_str(),buf.size()+1);
}

int main(int argc, char *argv[])
{
    int listenfd,connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    time_t ticks;

    //listenfd = socket(AF_INET,SOCK_STREAM,0);
    listenfd = Socket(AF_INET,SOCK_STREAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8000);

    bind(listenfd,(SA*)&servaddr,sizeof(servaddr));

    listen(listenfd,8);

    /*
    for (;;) {
        connfd = accept(listenfd,(sockaddr*)nullptr,nullptr);
        ticks = time(nullptr);
        snprintf(buff,sizeof(buff),"%.24s\r\n",ctime(&ticks));
        write(connfd,buff,strlen(buff));
        close(connfd);
    }*/
    /*
    for (;;)
    {

        if(!connfd)
            connfd = accept(listenfd,(SA*)nullptr,nullptr);
        int n = read(connfd,buff,MAXLINE);
        buff[n] = 0;
        if(n)
            std::cout<<buff<<std::endl;

        std::string sendMessage;
        std::cin>>sendMessage;
        write(connfd,sendMessage.c_str(),sendMessage.size());
        if(sendMessage=="#")
            close(connfd);
    }*/
    connfd = accept(listenfd,(SA*)nullptr,nullptr);
    std::thread recv(Read,connfd);
    std::thread send(Send,connfd);
    recv.join();
    send.join();
    close(connfd);
    close(listenfd);
    return 0;
}
