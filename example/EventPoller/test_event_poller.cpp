#include <sys/socket.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include "EventPoller.h"

int init_sock(char *port)
{
    struct addrinfo hint, *result;
    memset(&hint, 0, sizeof(addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int res = getaddrinfo(nullptr, port, &hint, &result);
    if(res == -1)
    {
        perror("error: can not get address info\n");
        exit(-1);
    }

    int fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(fd == -1)
    {
        perror("error: can not get socket descriptor!\n");
        exit(-1);
    }

    res = bind(fd, result->ai_addr, result->ai_addrlen);
    if(res == -1)
    {
        perror("error: can not bind the socket!\n");
        exit(-1);
    }

    freeaddrinfo(result);
    return fd;
}

static int set_socket_non_block(int sfd)
{
    int flags, res;
    flags = fcntl(sfd, F_GETFL);
    if (flags == -1)
    {
        perror("error : cannot get socket flags!\n");
        exit(1);
    }

    flags |= O_NONBLOCK;
    res    = fcntl(sfd, F_SETFL, flags);
    if (res == -1)
    {
        perror("error : cannot set socket flags!\n");
        exit(1);
    }

    return 0;
}

int main()
{
    int fd = init_sock();
    int epfd = epoll_create(1);
    epoll_ctl(epfd, fd, EPOLL_CTL_ADD, ev);

    epoll_event events[10];

    while (1)
    {
        int nready = epoll_wait(epfd, events, 10, 5);
        for (int i = 0; i < nready; ++i)
        {
            // 处理io
            if(events[i].events == EPOLLIN)
            {
                ;
            }
        }
    }
    return 0;
}