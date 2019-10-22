#include "tcp.h"
int main(int argc, char *argv[])
{
    int sockfd,n;
    char recvline[MAXLINE];
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
    {
        std::cout<<"socket error"<<std::endl;
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
    }

    /*
    while((n=read(sockfd,recvline,1024))>0){
        recvline[n] = 0;
        //fputs(recvline,stdout);
        std::cout<<recvline<<std::endl;
    }
    exit(0);*/
    while(true)
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
    }
    return 0;
}
