#ifndef TCP_H
#define TCP_H
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <thread>

#define MAXLINE 1024
#define SA sockaddr

void Listen(int fd,int backlog)
{
    char* ptr;

    if((ptr = getenv("LISTENQ")) != nullptr)
    {
        backlog = atoi(ptr);
    }
    if(listen(fd,backlog) < 0)
    {
        std::cout<<"Listen error!"<<std::endl;
    }
}

int Socket(int family,int type,int protocol)
{
    int n;

    if((n = socket(family,type,protocol)) < 0)
    {
        std::cout << "Create socket error!" << std::endl;
    }
    return n;
}

/*从一个描述符读n字节*/
ssize_t readn(int fd,void* vptr,size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = (char*)vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nread = read(fd,ptr,nleft)) < 0)
        {
            if(errno == EINTR)
            {
                nread = 0;
            }
            else
            {
                return -1;
            }
        }
        else if(nread == 0)
        {
            break;
        }

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);
}

/*从一个描述符写n字节*/
int writen(int fd,const void* vptr,size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char* ptr;

    ptr = (const char*)vptr;
    nleft = n;
    while(nleft > 0)
    {
        if((nwritten = write(fd,ptr,nleft)) <= 0)
        {
            if(nwritten < 0 && errno == EINTR)
            {
                nwritten = 0;
            }
            else
            {
                return -1;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}


static int read_cnt;
static char* read_ptr;
static char read_buf[MAXLINE];
static ssize_t readlineS(int fd,char *ptr)
{
    if(read_cnt <= 0)
    {
again:
        if((read_cnt = read(fd,read_buf,sizeof(read_buf))) < 0)
        {
            if(errno == EINTR)
            {
                goto again;
            }
            return -1;
        }
        else if(read_cnt == 0)
        {
            return 0;
        }
        read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readlinebuf(void** vptrptr)
{
    if(read_cnt)
    {
        *vptrptr = read_ptr;
    }
    return read_cnt;
}

/*从一个描述符中读取文本行,一次1字节*/
ssize_t readline(int fd,void* vptr,size_t maxlen)
{
    size_t n;
    ssize_t rc;
    char c,*ptr;

    ptr = (char*)vptr;
    for(n = 1; n < maxlen; n++)
    {
again:
        if((rc = readlineS(fd,&c)) == 1)
        {
            *ptr++ = c;
            if(c == '\n')
            {
                break;
            }
        }
        else if(rc == 0)
        {
            *ptr = 0;
            return n-1;
        }
        else
        {
            if(errno == EINTR)
            {
                goto again;
            }
            return -1;
        }
    }
    *ptr = 0;
    return n;
}
#endif
