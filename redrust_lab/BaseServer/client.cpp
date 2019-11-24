#include "tcp.h"

void Read(int connfd)
{
    char temp[1024] = {0};
    int n = readn(connfd,temp,MAXLINE);
    if(n<=0){
        return ;
    }
    temp[n] = '\0';
    std::cout<<temp<<std::endl;
}
void Send(int connfd){
    std::string buf;
    std::cin>>buf;
    int n = writen(connfd,buf.c_str(),buf.size()+1);
}


int main(int argc, char *argv[])
{
    int sockfd,n;
    struct sockaddr_in servaddr;

    if((sockfd = Socket(AF_INET,SOCK_STREAM,0))<0)
    {
        std::cout<<"socket error"<<std::endl;
        return -1;
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8000);

    if((inet_pton(AF_INET,argv[1],&servaddr.sin_addr))<=0)
    {
        std::cout<<"socket error"<<std::endl;
    }
    if(connect(sockfd,(sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        std::cout<<"connect error"<<std::endl;
        return -2;
    }

    /*
    while((n=read(sockfd,recvline,1024))>0){
        recvline[n] = 0;
        //fputs(recvline,stdout);
        std::cout<<recvline<<std::endl;
    }
    exit(0);*/
    /*while(true)
    {
        std::string sendMessage;
        std::cin>>sendMessage;
        if(sendMessage=="#")
        {
            exit(0);
        }
        write(sockfd,sendMessage.c_str(),sendMessage.size());
        n=read(sockfd,recvline,MAXLINE);
        recvline[n]=0;
        std::cout<<recvline<<std::endl;
    }*/
    std::thread send(Send,sockfd);
    std::thread recv(Read,sockfd);
    /*for(;;){
        recv.join();
        send.join();
    }*/
    getchar();
    return 0;
}
